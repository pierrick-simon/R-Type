# Writing a game without a `main`: `engine-game`

> 🌐 **Language:** [Français](ENGINE_GAME.fr.md) · English

`engine-game` is the library that provides the `main` instead of the game. The game gives two things:

- **a C++ variable, `RType::GameConfig`**: the window, the frame rate, the engine modules to load and when to stop;
- **Lua scripts**: they create the entities, give them components and register the systems.

The engine does the rest: it creates the `GameManager`, loads the modules, runs every script, then runs the loop.

- **Audience:** anyone making a game with the engine.
- **To write the scripts**, see [the Lua scripting guide](LUA_GUIDE.en.md).
- **For the ECS in detail** (entities, components, systems, views), see [the ECS user guide](ECS_GUIDE.en.md).

> [!TIP]
> Every example in this guide was compiled and run against the current engine. The error messages in [section 8](#8-errors-and-how-to-fix-them) were reproduced with `g++-14`.

---

## Contents

1. [What the engine's `main` does](#1-what-the-engines-main-does)
2. [`GameInfo`: what the game provides](#2-gameinfo-what-the-game-provides)
3. [The modules](#3-the-modules)
4. [Setting up the game](#4-setting-up-the-game)
5. [A complete game](#5-a-complete-game)
6. [Debug mode](#6-debug-mode)
7. [Do / don't](#7-do--dont)
8. [Errors and how to fix them](#8-errors-and-how-to-fix-them)

---

## 1. What the engine's `main` does

The `main` lives in [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp) and calls `RType::Game::run()`. In order, it:

1. creates the `GameManager` (in debug mode if the engine is built with `__DEBUG__`, see [section 6](#6-debug-mode));
2. loads the modules asked for in `GameConfig.modules`, plus `Clock`, which is always loaded (see [section 3](#3-the-modules));
3. loads every `.lua` file of the `TestLua/` folder and its subfolders, **in alphabetical order of their path**, and calls the `start(gm)` function of each one (see [the Lua guide](LUA_GUIDE.en.md));
4. runs `gm.run(GameConfig.windowFPS, GameConfig.runningCondition)`: the loop goes on as long as `runningCondition()` returns `true`;
5. if an `RType::EngineException` comes up, from the engine or from a script, prints its message and exits with code `84`.

The `TestLua/` folder is looked for **in the directory the game is started from**.

---

## 2. `GameInfo`: what the game provides

The [`Game.hpp`](../Engine/Game/include/Game.hpp) header declares the struct and the variable:

```cpp
namespace RType {

    struct GameInfo {
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
        std::list<std::string> modules;
    };

    extern GameInfo GameConfig;

}
```

| Field | Role | Default |
|---|---|---|
| `runningCondition` | Called before each frame: the loop goes on as long as it returns `true` | **none, must be provided** |
| `windowFPS` | Frames per second of the loop | `60` |
| `windowWidth`, `windowHeight` | Window size, in pixels, used by the `Window` module | `800` × `600` |
| `windowName` | Window title, used by the `Window` module | `"My Game"` |
| `modules` | The engine modules to load, see [section 3](#3-the-modules) | empty (only `Clock`) |

The game **defines** `RType::GameConfig` in **a single** one of its `.cpp` files, with C++20 designated initializers. Fields left out keep their default value:

```cpp
#include "Game.hpp"

RType::GameInfo RType::GameConfig = {
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
    .modules = {"Window", "Keyboard"},
};
```

Fields must be written **in the struct's order**: `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`, `modules`. You can skip some, not swap them.

---

## 3. The modules

A module is an engine feature the game turns on by name. Each one creates its entity and registers its system, **before** the scripts run, so the scripts' systems always come after them.

| Module | What it creates | Needs |
|---|---|---|
| `Window` | an entity with a `Window` (size and title from `GameConfig`), and the system that clears and refreshes it each frame | |
| `Keyboard` | an entity with a `KeyboardManager`, and the system that reads the keys | `Window` |
| `Mouse` | an entity with a `MouseManager`, and the system that reads the mouse | `Window` |
| `Clock` | an entity with the `MasterClock`, and the system that advances it | always loaded |

- **`Clock` is always loaded**, even if `modules` does not list it: the engine's movement and animation systems rely on it.
- **A module is loaded only once**, even if it is listed twice.
- **The order is kept**: the game's modules first, in its order, then `Clock`.
- **A missing dependency stops the game** before anything runs, for instance `Keyboard` without `Window` (see [section 8](#8-errors-and-how-to-fix-them)).

---

## 4. Setting up the game

### The folder

```
Games/
└── MyGame/
    ├── CMakeLists.txt
    ├── src/
    │   └── Setup.cpp        # only the definition of RType::GameConfig
    └── TestLua/
        ├── game.lua         # the scripts, loaded in alphabetical order
        └── sprites/
            └── ship.png
```

No `Main.cpp`: `engine-game` provides it.

### The `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(my_game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(my_game src/Setup.cpp)
target_link_libraries(my_game PRIVATE engine-game)
```

**`engine-game` is enough**: it brings the ECS, the 2D and 3D graphics, the audio and the Lua interpreter, with their include directories.

Then add the folder to [`Games/CMakeLists.txt`](../Games/CMakeLists.txt):

```cmake
add_subdirectory(MyGame)
```

and the binary's name to the [`.gitignore`](../.gitignore).

### Building and running

```sh
cmake -S . -B build
cmake --build build --target my_game -j
cd Games/MyGame && ../my_game
```

The binary is placed in `Games/`. It must be **started from the folder that holds `TestLua/`**: the scripts, and the paths they give (images, fonts…), are read relative to that folder.

---

## 5. A complete game

A ship crossing the window and bouncing off its edges. `Escape` or the close button quit the game.

```cpp
// src/Setup.cpp
#include "Game.hpp"

RType::GameInfo RType::GameConfig = {
    .runningCondition = [] { return !WindowShouldClose(); },
    .windowName = "Bounce",
    .modules = {"Window", "Keyboard"},
};
```

```lua
-- TestLua/game.lua
local WIDTH, HEIGHT = 800, 600

local function bounce(gm, e, pos, vel)
    if pos.x < 0 or pos.x > WIDTH - 32 then vel.x = -vel.x end
    if pos.y < 0 or pos.y > HEIGHT - 32 then vel.y = -vel.y end
end

function start(gm)
    local ship = gm:createEntity()
    gm:addComponent(ship, 'Position2D', {x = 100, y = 100})
    gm:addComponent(ship, 'Velocity2D', {x = 250, y = 180})
    gm:addComponent(ship, 'Texture2D', {path = 'TestLua/sprites/ship.png'})
    gm:addComponent(ship, 'Sprite2D', {scale = 2})

    gm:createSystem({'Position2D', 'Velocity2D'}, bounce)
    gm:addSystem('MasterVelocity2D')
    gm:addSystem('DrawSprite2D')
end
```

- **`Setup.cpp`** only turns on the window and the keyboard. The `Window` module already clears the window each frame.
- **`start`** runs once: it creates the ship, then registers three systems, which run each frame in this order. `bounce` is the game's own; `MasterVelocity2D` (movement) and `DrawSprite2D` (drawing) come from the engine.

Everything the script can do is in [the Lua scripting guide](LUA_GUIDE.en.md).

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
[GameManager] addComponent<RType::MasterClock> on entity 0
[GameManager] createSystem with ( RType::MasterClock ) -> system 0
```

The macro must apply to `engine-game`, which holds the `main`: a `target_compile_definitions` on the game's target is not enough.

---

## 7. Do / don't

### Do

- **Define `RType::GameConfig` in a single `.cpp`**, ideally a `Setup.cpp` that holds only that.
- **Always provide `runningCondition`**.
- **Link only `engine-game`** in the game's `CMakeLists.txt`.
- **List `Window` in `modules`** as soon as the game draws something or reads the keyboard or the mouse.
- **Start the game from the folder that holds `TestLua/`**.
- **Write `GameInfo`'s fields in the struct's order**.

### Don't

- **Don't write a `main`** in the game: it would silently replace the engine's.
- **Don't define `RType::GameConfig` in a header**, nor in two `.cpp` files.
- **Don't list `Clock` in `modules`**: it is always loaded.
- **Don't read `RType::GameConfig` from another global variable's constructor**: the initialization order of globals across files is not defined.

---

## 8. Errors and how to fix them

### `undefined reference to 'RType::GameConfig'`

```
Game.cpp:(.text+0xa58): undefined reference to `RType::GameConfig'
```

**Cause:** the game does not define `RType::GameConfig`.
**Fix:** add the definition in one of the game's `.cpp` files (see [section 2](#2-gameinfo-what-the-game-provides)), and check that this file is listed in `add_executable`.

### `multiple definition of 'RType::GameConfig'`

**Cause:** `RType::GameConfig` is defined in two `.cpp` files, or in a header included by several `.cpp` files.
**Fix:** keep a single definition, in a single `.cpp`.

### `designator order for field ... does not match declaration order`

```
error: designator order for field 'RType::GameInfo::runningCondition' does not match declaration order in 'RType::GameInfo'
```

**Cause:** `GameInfo`'s fields are not written in the struct's order.
**Fix:** put them back in the order `runningCondition`, `windowFPS`, `windowWidth`, `windowHeight`, `windowName`, `modules`.

### `terminate called after throwing an instance of 'std::bad_function_call'`

```
terminate called after throwing an instance of 'std::bad_function_call'
  what():  bad_function_call
```

**Cause:** `runningCondition` is not provided: the loop calls an empty `std::function`.
**Fix:** define `.runningCondition`, for instance `[] { return !WindowShouldClose(); }`.

### `Engine Error: No such module: ...` (exit code `84`)

```
Engine Error: No such module: Windw
```

**Cause:** a name in `modules` is not one of the modules of [section 3](#3-the-modules), often because of a typo.
**Fix:** correct the name. The module names are case sensitive.

### `Engine Error: Module ... requires module ...` (exit code `84`)

```
Engine Error: Module Keyboard requires module Window
```

**Cause:** a module is listed without the module it needs.
**Fix:** add the missing module to `modules`.

### `Engine Error: Script error: ...` (exit code `84`)

**Cause:** a script has a mistake, or calls the engine with wrong arguments. The message gives the file and the line.
**Fix:** see [the errors of the Lua guide](LUA_GUIDE.en.md#9-errors-and-how-to-fix-them).

### Nothing happens: no script is run

**Cause:** the game is not started from the folder that holds `TestLua/`. The engine finds no script, and runs an empty loop without error.
**Fix:** `cd` into that folder before starting the game (see [section 4](#building-and-running)).

### The window does not open

**Cause:** `Window` is not listed in `modules`.
**Fix:** add `"Window"` to `modules`.
