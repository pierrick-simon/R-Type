# Writing a game without a `main`: `engine-game`

> 🌐 **Language:** [Français](ENGINE_GAME.fr.md) · English

`engine-game` is the library that provides the `main` instead of the game. The game only writes its init functions and its systems, then puts them in a single variable, `RType::Game`. The engine does the rest: it creates the `GameManager`, the window, the keyboard and the clock, calls the game's functions in order, then runs the loop.

- **Audience:** anyone making a game with the engine.
- **For the ECS in detail** (entities, components, systems, views), see [the ECS user guide](ECS_GUIDE.en.md).
- **For the provided components** (images, animations, text, sound), see [the game making guide](GAME_GUIDE.en.md).

> [!TIP]
> Every example in this guide was compiled and run against the current engine. The error messages in [section 8](#8-errors-and-how-to-fix-them) were reproduced with `g++-14`.

---

## Contents

1. [What the engine's `main` does](#1-what-the-engines-main-does)
2. [`GameInfo`: what the game provides](#2-gameinfo-what-the-game-provides)
3. [Setting up the game](#3-setting-up-the-game)
4. [A complete game in one file](#4-a-complete-game-in-one-file)
5. [Writing the inits](#5-writing-the-inits)
6. [Debug mode](#6-debug-mode)
7. [Do / don't](#7-do--dont)
8. [Errors and how to fix them](#8-errors-and-how-to-fix-them)

---

## 1. What the engine's `main` does

The `main` lives in [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp). In order, it:

1. creates the `GameManager` (in debug mode if the engine is built with `__DEBUG__`, see [section 6](#6-debug-mode));
2. creates three entities, each with **a single** engine component:
   - a `Window` with the size and title given by `RType::Game`;
   - a `KeyboardManager`;
   - a `MasterClock`;
3. calls every function in `RType::Game.inits`, **in list order**;
4. runs `gm.run(RType::Game.windowFPS, RType::Game.runningCondition)`: the loop goes on as long as `runningCondition()` returns `true`;
5. if an `RType::EngineException` comes up, prints its message and exits with code `84`.

The `main` **creates the components but registers no system**. The game registers, in its inits, the engine systems it needs (clock, keyboard, window clearing…), see [section 5](#registering-the-engine-systems).

---

## 2. `GameInfo`: what the game provides

The [`Game.hpp`](../Engine/Game/include/Game.hpp) header declares the struct and the variable:

```cpp
namespace RType {

    struct GameInfo {
        std::list<std::function<void(RType::GameManager &)>> inits = {};
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
    };

    extern GameInfo Game;

}
```

| Field | Role | Default |
|---|---|---|
| `inits` | Functions called once at startup, in order | empty |
| `runningCondition` | Called before each frame: the loop goes on as long as it returns `true` | **none, must be provided** |
| `windowFPS` | Frames per second of the loop | `60` |
| `windowWidth`, `windowHeight` | Window size, in pixels | `800` × `600` |
| `windowName` | Window title | `"My Game"` |

The game **defines** `RType::Game` in **a single** one of its `.cpp` files, with C++20 designated initializers. Fields left out keep their default value:

```cpp
RType::GameInfo RType::Game = {
    .inits = {createWorld, createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "My game",
};
```

Fields must be written **in the struct's order**: `inits`, `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`. You can skip some, not swap them.

---

## 3. Setting up the game

### The folder

```
Games/
└── MyGame/
    ├── CMakeLists.txt
    ├── include/
    │   └── MyGame.hpp     # components and function declarations
    └── src/
        ├── Ball.cpp       # functions that create the entities
        ├── Systems.cpp    # the systems
        └── Game.cpp       # only the definition of RType::Game
```

No `Main.cpp`: `engine-game` provides it.

### The `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(my_game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(my_game
        src/Ball.cpp
        src/Systems.cpp
        src/Game.cpp
)
target_include_directories(my_game PRIVATE include)
target_link_libraries(my_game PRIVATE engine-game)
```

**`engine-game` is enough**: it brings `engine-system` and `engine-graphical`, with their include directories. If the game is a single file with no `include/` folder, the `target_include_directories` line is not needed.

Then add the folder to [`Games/CMakeLists.txt`](../Games/CMakeLists.txt):

```cmake
add_subdirectory(MyGame)
```

and the binary's name to the [`.gitignore`](../.gitignore), next to `r-type_client` and `r-type_server`.

### Building and running

```sh
cmake -S . -B build
cmake --build build --target my_game -j
./Games/my_game
```

The binary is placed directly in `Games/`.

### Several files

Functions are declared in the header and defined in their own files. `Game.cpp` only holds the definition of `RType::Game`:

```cpp
// include/Bounce.hpp
#include "GameManager.hpp"

namespace Bounce {

    struct Ball {
        float radius = 20.f;
    };

    void createBall(RType::GameManager &gm);
    void createSystems(RType::GameManager &gm);

} // namespace Bounce
```

```cpp
// src/Game.cpp
#include "Bounce.hpp"
#include "Game.hpp"
#include "Window.hpp"

RType::GameInfo RType::Game = {
    .inits = {Bounce::createBall, Bounce::createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowWidth = 1280,
    .windowHeight = 720,
    .windowName = "Bounce",
};
```

---

## 4. A complete game in one file

A ball bouncing off the window's edges. `Space` reverses its direction, `Escape` or the close button quit the game.

```cpp
#include <algorithm>
#include <cmath>

#include "Clock.hpp"
#include "Game.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Position2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace {

    constexpr float WIDTH = 800.f;
    constexpr float HEIGHT = 600.f;

    struct Ball {
        float radius = 20.f;
    };

    void createBall(RType::GameManager &gm)
    {
        RType::Entity ball = gm.createEntity();

        gm.addComponent<Ball>(ball);
        gm.addComponent<RType::Position2D>(ball, RType::Position2D {400.f, 300.f});
        gm.addComponent<RType::Velocity2D>(ball, RType::Velocity2D {250.f, 180.f});
    }

    void bounce(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, b, pos, vel] : gm.view<Ball, Position2D, Velocity2D>().getVector()) {
            if (pos.x < b.radius)
                vel.x = std::abs(vel.x);
            if (pos.x > WIDTH - b.radius)
                vel.x = -std::abs(vel.x);
            if (pos.y < b.radius)
                vel.y = std::abs(vel.y);
            if (pos.y > HEIGHT - b.radius)
                vel.y = -std::abs(vel.y);
        }
    }

    void reverseOnSpace(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, k] : gm.view<KeyboardManager>().getVector()) {
            if (std::ranges::find(k.keyPressed, Key::Value::Space) == k.keyPressed.end())
                continue;
            for (auto [_, b, vel] : gm.view<Ball, Velocity2D>().getVector()) {
                vel.x = -vel.x;
                vel.y = -vel.y;
            }
        }
    }

    void draw(RType::GameManager &gm)
    {
        using namespace RType;

        for (auto [_, b, pos] : gm.view<Ball, Position2D>().getVector())
            DrawCircleV({pos.x, pos.y}, b.radius, RED);
    }

    void createSystems(RType::GameManager &gm)
    {
        using namespace RType;

        gm.createSystem<MasterClock>(MasterClock::systemMasterClock);
        gm.createSystem<KeyboardManager>(KeyboardManager::systemKeyboardManager);
        gm.createSystem<>(reverseOnSpace);
        gm.createSystem<>(bounce);
        gm.createSystem<MasterClock>(Velocity2D::systemMasterVelocity2D);
        gm.createSystem<Window>(Window::systemClearWindow);
        gm.createSystem<>(draw);
    }

} // namespace

RType::GameInfo RType::Game = {
    .inits = {createBall, createSystems},
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
};
```

- **`createBall`** creates the game's entities, **`createSystems`** registers every system, once.
- **`bounce`, `reverseOnSpace` and `draw`** only take the `GameManager`: registered with `gm.createSystem<>(...)`, they are called once per frame and find their entities through views.

---

## 5. Writing the inits

### An init is called only once

Each function in `inits` is called **once**, at startup. It is used to create entities and register systems. What must happen **every frame** (movement, game logic, drawing) goes into a system:

```cpp
// bounce is called only once: the ball never bounces
.inits = {createBall, bounce},

// bounce becomes a system: it is called every frame
.inits = {createBall, [](RType::GameManager &gm) { gm.createSystem<>(bounce); }},
```

### Registering the engine systems

The `main` creates the `Window`, the `KeyboardManager` and the `MasterClock`, but does not register their systems. Without them, the clock stays at zero, the keyboard stays empty and the window is not refreshed. A game init must therefore register them:

```cpp
gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);
gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
gm.createSystem<RType::Window>(RType::Window::systemClearWindow);
```

Systems run **in the order they are created**, so the order of the inits matters. The recommended order:

1. the clock and the keyboard;
2. the game logic;
3. physics (`Velocity2D::systemMasterVelocity2D`);
4. `Window::systemClearWindow`, then drawing.

### Reading the window, the keyboard and the clock

These three components are **each on their own entity**. A view asking for several of them at once therefore finds no entity. Fetch them with one view each:

```cpp
void updateGame(RType::GameManager &gm)
{
    using namespace RType;

    auto keyboards = gm.view<KeyboardManager>().getVector();
    auto clocks = gm.view<MasterClock>().getVector();

    if (keyboards.empty() || clocks.empty())
        return;
    auto &k = std::get<1>(keyboards.front());
    auto &c = std::get<1>(clocks.front());
    float dt = float(c.getElapsed().count()) / MILLI_IN_SEC;
    // ...
}
```

### A function that takes other parameters

Inits all have the signature `void(RType::GameManager &)`. For a function that takes other parameters, wrap it in a lambda that fixes them:

```cpp
void createEnemy(RType::GameManager &gm, float x, float y);

RType::GameInfo RType::Game = {
    .inits = {
        createSystems,
        [](RType::GameManager &gm) { createEnemy(gm, 100.f, 50.f); },
        [](RType::GameManager &gm) { createEnemy(gm, 300.f, 50.f); },
    },
    .runningCondition = [] { return !WindowShouldClose(); },
};
```

### Stopping the game

`runningCondition` is called before each frame. `[] { return !WindowShouldClose(); }` stops the game when the window is closed or `Escape` is pressed. It can also read some global game state, for instance a boolean set to `true` by a system when the game is over.

---

## 6. Debug mode

When the engine is built with the `__DEBUG__` macro, the `main` creates the `GameManager` in debug mode: every operation is written to `std::clog`.

```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D__DEBUG__"
cmake --build build --target my_game -j
```

```
[GameManager] debug mode enabled
[GameManager] createEntity -> entity 0
[GameManager] addComponent<RType::Window> on entity 0
[GameManager] createEntity -> entity 1
[GameManager] addComponent<RType::KeyboardManager> on entity 1
[GameManager] createEntity -> entity 2
[GameManager] addComponent<RType::MasterClock> on entity 2
[GameManager] createSystem with ( RType::MasterClock ) -> system 0
```

The macro must apply to `engine-game`, which holds the `main`: a `target_compile_definitions` on the game's target is not enough.

---

## 7. Do / don't

### Do

- **Define `RType::Game` in a single `.cpp`**, ideally a `Game.cpp` that holds only that.
- **Always provide `runningCondition`**.
- **Link only `engine-game`** in the game's `CMakeLists.txt`.
- **Register the clock, the keyboard and `Window::systemClearWindow`** in an init.
- **Put per-frame logic in systems**, registered once in an init.
- **Fetch the window, the keyboard and the clock with one view each**.
- **Write `GameInfo`'s fields in the struct's order**.

### Don't

- **Don't write a `main`** in the game: it would silently replace the engine's.
- **Don't define `RType::Game` in a header**, nor in two `.cpp` files.
- **Don't create a `Window` or a `MasterClock`** in an init: the engine already creates them.
- **Don't put a function meant to run every frame in `inits`**.
- **Don't throw anything other than an `EngineException`** from an init or a system: the `main` does not catch it and the program aborts.
- **Don't read `RType::Game` from another global variable's constructor**: the initialization order of globals across files is not defined.

---

## 8. Errors and how to fix them

### `undefined reference to 'RType::Game'`

```
Main.cpp:22: undefined reference to `RType::Game'
```

**Cause:** the game does not define `RType::Game`.
**Fix:** add the definition in one of the game's `.cpp` files (see [section 2](#2-gameinfo-what-the-game-provides)), and check that this file is listed in `add_executable`.

### `multiple definition of 'RType::Game'`

```
multiple definition of `RType::Game'; first defined here
```

**Cause:** `RType::Game` is defined in two `.cpp` files, or in a header included by several `.cpp` files.
**Fix:** keep a single definition, in a single `.cpp`. The engine's `Game.hpp` header takes care of the declaration.

### `designator order for field ... does not match declaration order`

```
error: designator order for field 'RType::GameInfo::windowFPS' does not match declaration order in 'RType::GameInfo'
```

**Cause:** `GameInfo`'s fields are not written in the struct's order.
**Fix:** put them back in the order `inits`, `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`.

### `undefined reference to 'main'`

**Cause:** the game is not linked to `engine-game`.
**Fix:** `target_link_libraries(my_game PRIVATE engine-game)`.

### `Window.hpp: No such file or directory` (or `KeyboardManager.hpp`, `Clock.hpp`…)

**Cause:** the game is linked only to `engine-system`, or to no engine library at all.
**Fix:** link `engine-game`, which brings the includes of `engine-system` and `engine-graphical`.

### `terminate called after throwing an instance of 'std::bad_function_call'`

```
terminate called after throwing an instance of 'std::bad_function_call'
  what():  bad_function_call
```

**Cause:** `runningCondition` is not provided: the loop calls an empty `std::function`.
**Fix:** define `.runningCondition`, for instance `[] { return !WindowShouldClose(); }`.

### `terminate called after throwing an instance of '...'`

**Cause:** an init or a system threw an exception that is not an `RType::EngineException` (`std::runtime_error`, `std::out_of_range`…). The `main` only catches `EngineException`s.
**Fix:** catch the exception in the game, or throw an `RType::EngineException` instead.

### `Engine Error: Window can be created only once` (exit code `84`)

**Cause:** an init creates a `Window`, while the `main` already created one.
**Fix:** remove that creation. Size and title are set with `windowWidth`, `windowHeight` and `windowName`.

### The game ignores `RType::Game`

**Symptom:** the game builds and starts, but no window opens and the inits are not called.
**Cause:** the game has a `main`. The linker uses it instead of `engine-game`'s, without any error.
**Fix:** remove the game's `main`.

### Nothing moves

**Possible causes:**

- **The clock system is not registered**: `getElapsed()` stays at zero, so every velocity moves things by nothing. Register `MasterClock::systemMasterClock`.
- **The movement system is not registered**: register `Velocity2D::systemMasterVelocity2D`.
- **The logic is in `inits` instead of a system**: it is called only once. Register it with `gm.createSystem<>(...)`.
- **A view asks for several engine components at once**, for instance `view<KeyboardManager, MasterClock>()`: they are on different entities, the view is empty and the system does nothing. Use one view per component ([section 5](#reading-the-window-the-keyboard-and-the-clock)).

### The keyboard does not respond

**Possible causes:**

- `KeyboardManager::systemKeyboardManager` is not registered.
- `Window::systemClearWindow` is not registered: raylib only reads keyboard and window events at the end of each draw.
- The system reading the keyboard is registered **before** `systemKeyboardManager`: it reads the previous frame's keys.

### The window stays black or frozen

**Cause:** `Window::systemClearWindow` is not registered, or the drawing systems are registered without it.
**Fix:** register `Window::systemClearWindow`, then the drawing systems after it.
