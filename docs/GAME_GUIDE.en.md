# Making a game with the engine

> 🌐 **Language:** [Français](GAME_GUIDE.fr.md) · English

This guide explains how to **make a game** on top of the engine: where to put the code, how to build it, how to organize it, and how to use the provided components (window, images, animations, movement, keyboard, mouse, text, sound). It ends with a list of **things to do** and **things not to do**.

- **Audience:** anyone starting a game (or a part of the R-Type) with the engine.
- **For the ECS in detail** (entities, components, systems, views, frame), see [the ECS user guide](ECS_GUIDE.en.md). This guide assumes you have read at least its [section 1](ECS_GUIDE.en.md#1-the-essentials-in-30-seconds).
- **To understand the engine's design choices**, see [the ECS design notes](ECS_DESIGN.en.md).

Throughout the guide, we build a small shoot'em up: a ship that moves with the keyboard and shoots, enemies coming from the right, collisions and a score.

> [!TIP]
> Every example in this guide was compiled against the current engine, and the complete game was run with them.

---

## Table of contents

1. [What the engine provides](#1-what-the-engine-provides)
2. [Creating the game folder](#2-creating-the-game-folder)
3. [The `main`: window, input, loop](#3-the-main-window-input-loop)
4. [Organizing the code](#4-organizing-the-code)
5. [The order of systems](#5-the-order-of-systems)
6. [Basic features](#6-basic-features)
7. [Do](#7-do)
8. [Don't](#8-dont)
9. [Current engine limitations](#9-current-engine-limitations)
10. [Going further](#10-going-further)

---

## 1. What the engine provides

The engine (`Engine/`) is split into several libraries:

| Library | Contents | Header folder |
|---|---|---|
| `engine-system` | The ECS (`GameManager`) and the basic components: position, clock… | `Engine/System/include/` (and its `BuiltIn/`, `Components/`, `System/` subfolders) |
| `engine-physics` | Velocity, friction, gravity and rotation, in 2D and 3D | `Engine/Physics/include/` |
| `engine-graphical2D` | Window, images, animations, text, keyboard, mouse (based on raylib) | `Engine/Graphical2D/include/` |
| `engine-graphical3D` | 3D models, 3D sprites and the camera (based on raylib) | `Engine/Graphical3D/include/` |
| `engine-audio` | Sounds and the point that hears them (based on miniaudio) | `Engine/Audio/include/` |
| `engine-network` | Networking: UDP client and server (based on Asio) | `Engine/Network/include/` |

Each feature is a **component** to attach to an entity, usually along with a **static `systemXxx` function** to register as a system. The engine registers no system by itself: the game has to do it, in the right order (see [section 5](#5-the-order-of-systems)).

| Component | Library | Role | Provided system (requested components) |
|---|---|---|---|
| `Position2D` | system | Position in pixels, `(0, 0)` at the top-left | — |
| `MasterClock` | system | The game's clock: time elapsed since the previous frame, one for the whole game | `MasterClock::systemMasterClock` (`MasterClock`) |
| `Clock` | system | An entity's own clock, to slow it down or speed it up alone | `Clock::systemClock` (`Clock`) |
| `Origin2D` | system | Pivot point of a drawable (used in `Sprite2D`, `SpriteSheet2D`, `Text2D`) | — |
| `Size2D` | system | Integer size, **in the order `{height, width}`** | — |
| `Velocity2D` | physics | Velocity in pixels per second | `Velocity2D::systemMasterVelocity2D` (`MasterClock`), or `Velocity2D::systemVelocity2D` (`Velocity2D, Position2D, Clock`) |
| `GravityField2D` | physics | Acceleration applied to the velocity (`x`, `y` = direction, `g` = strength in pixels/s²) | `GravityField2D::systemMasterGravityField2D` (`MasterClock`), or `GravityField2D::systemGravityField2D` (`GravityField2D, Velocity2D, Clock`) |
| `Friction2D` | physics | Friction applied to the velocity | `Friction2D::systemMasterFriction2D` (`MasterClock`), or `Friction2D::systemFriction2D` (`Friction2D, Velocity2D, Clock`) |
| `Window` | graphical2D | The window | its `beginDrawing()` and `endDrawing()` methods, around the drawing systems (see [section 5](#5-the-order-of-systems)) |
| `Texture2D` | graphical2D | An image loaded from a file | — |
| `Sprite2D` | graphical2D | Draws a whole texture | `Sprite2D::systemDrawSprite2D` (`Sprite2D, Texture2D, Position2D`) |
| `SpriteSheet2D` | graphical2D | Draws one tile of an animation sheet, and moves to the next one | `SpriteSheet2D::systemMasterAnimateSpriteSheet2D` (`MasterClock`) and `SpriteSheet2D::systemDrawSpriteSheet2D` (`SpriteSheet2D, Texture2D, Position2D`) |
| `Font` | graphical2D | A font loaded from a file | — |
| `Text2D` | graphical2D | A text to draw | `Text2D::systemDrawText2D` (`Text2D, Font, Position2D`) |
| `KeyboardManager` | graphical2D | The keyboard state (pressed, held and released keys) | `KeyboardManager::systemKeyboardManager` (`KeyboardManager`) |
| `MouseManager` | graphical2D | The mouse state (buttons, position, wheel) | `MouseManager::systemMouseManager` (`MouseManager`) |
| `Keybind` | graphical2D | Rebinds keys and buttons | `Keybind::systemUpdateKeybind` (`Keybind, MouseManager, KeyboardManager`) |
| `Listener` | audio | The point that hears the sounds, one for the whole game: every `Sound` is created from it | — |
| `Sound` | audio | A sound loaded from a file | `Sound::systemSound` (`Sound`) |

All these types live in the `RType` namespace. Their full documentation (Doxygen) is in their header.

---

## 2. Creating the game folder

### The layout

A game lives in its own folder under `Games/`, next to the R-Type:

```
Games/
└── MyGame/
    ├── CMakeLists.txt
    ├── assets/              # images, fonts, sounds
    ├── include/
    │   ├── Components.hpp   # the game's own components
    │   ├── Spawn.hpp        # the functions that create entities
    │   └── Systems.hpp      # the game's systems
    └── src/
        ├── Main.cpp
        ├── Spawn.cpp
        └── Systems.cpp
```

### The `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.25...4.0)
project(my_game CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRCS
        src/Main.cpp
        src/Spawn.cpp
        src/Systems.cpp
)

add_executable(my_game ${SRCS})
target_include_directories(my_game PRIVATE include)
target_link_libraries(my_game PRIVATE engine-system engine-physics engine-graphical2D engine-audio)
target_compile_definitions(my_game PRIVATE
        ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)
```

- **`engine-system`** provides the ECS and the basic components, **`engine-physics`** the movement, **`engine-graphical2D`** everything that is drawn or read from the keyboard and the mouse, **`engine-audio`** the sounds. A server that draws nothing does not need `engine-graphical2D` nor `engine-audio`.
- **`ASSETS_DIR`** is the absolute path of the `assets/` folder. File paths are relative to the folder the binary is launched from: with `ASSETS_DIR "player.png"`, the game finds its images wherever it is launched from.

Then add the folder to `Games/CMakeLists.txt`:

```cmake
add_subdirectory(MyGame)
```

### Building and running

Same as the rest of the project (see the [README](../README.md)):

```sh
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14
cmake --build build --target my_game -j
./Games/my_game
```

The binary is placed directly in `Games/`, like `r-type_client` and `r-type_server`.

---

## 3. The `main`: window, input, loop

```cpp
#include <iostream>
#include "Clock.hpp"
#include "Exceptions.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Listener.hpp"
#include "MouseManager.hpp"
#include "Spawn.hpp"
#include "Systems.hpp"
#include "Texture2D.hpp"
#include "Window.hpp"

int main()
{
    try {
        RType::GameManager gm;

        // 1. The window first: textures and fonts need it.
        RType::Entity screen = gm.createEntity();
        gm.addComponent<RType::Window>(screen, 1280, 720, "My game");

        // 2. Input, once for the whole game.
        RType::Entity input = gm.createEntity();
        gm.addComponent<RType::KeyboardManager>(input);
        gm.addComponent<RType::MouseManager>(input);

        // 3. The game's clock and the point that hears the sounds.
        RType::Entity world = gm.createEntity();
        gm.addComponent<RType::MasterClock>(world);
        gm.addComponent<RType::Listener>(world);

        // 4. Every system, before the loop.
        registerSystems(gm);

        // 5. The starting entities.
        spawnBackground(gm);
        spawnPlayer(gm, gm.getComponent<RType::Listener>(world));
        spawnScore(gm);
        spawnSpawner(gm);

        // 6. The loop: one frame per turn, until the window is closed.
        RType::Window &window = gm.getComponent<RType::Window>(screen);
        while (!window.shouldClose())
            gm.update();

        // 7. The textures before the window: raylib needs it to free them.
        RType::Texture2D::unloadTextures2D(gm);
    } catch (const RType::EngineException &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}
```

The key points:

- **The window is created first.** `Texture2D` and `Font` need it to load. And since `addComponent` constructs the component right away, the order of the lines matters.
- **Input has its own entity**, with a single `KeyboardManager` and a single `MouseManager`. The systems that react to the keyboard receive that `KeyboardManager` (see [Reading the keyboard and mouse](#reading-the-keyboard-and-mouse)).
- **The game's clock and the `Listener` are created once.** `MasterClock` gives every system the time of the frame, and the `Listener` is the point that hears the sounds: every `Sound` is created from it.
- **Every system is created before the loop**, never during `update()` (see [pitfall 2](ECS_GUIDE.en.md#pitfall-2-creating-a-system-during-update)).
- **`window.shouldClose()`** becomes true when the window is closed (close button or `Escape`).
- **`Texture2D::unloadTextures2D(gm)` runs before the end of `main`.** It destroys the entities that hold a texture while the window still exists: without it, the game crashes when it quits.
- **Every engine error derives from `RType::EngineException`**: a single `catch` clause is enough. The details are in [error handling](ECS_GUIDE.en.md#6-error-handling).

---

## 4. Organizing the code

### The game's components

The game's own components are plain data structs, grouped in a header:

```cpp
// include/Components.hpp
#include <chrono>

struct Player {
    float speed = 400.f;
};

struct Enemy {
    int hp = 3;
};

struct Bullet {};            // tag: "this entity is a bullet"

struct DestroyOffScreen {};  // tag: "destroy this entity when it leaves the screen"

struct Hitbox {
    float width = 0;
    float height = 0;
};

struct Score {
    int value = 0;
};

struct Spawner {
    std::chrono::milliseconds every = std::chrono::milliseconds(1500);
    std::chrono::milliseconds elapsed = std::chrono::milliseconds(0);
};
```

**Tags** (empty structs) are used to sort entities: a system on `Bullet` only sees bullets. Behaviors are combined this way, without a class hierarchy. The rules for writing a component are in [Writing a component](ECS_GUIDE.en.md#writing-a-component).

### One function per kind of entity

Each "kind" of entity is created by a `spawnXxx` function, which attaches all its components:

```cpp
// src/Spawn.cpp
RType::Entity spawnPlayer(RType::GameManager &gm, RType::Listener &listener)
{
    RType::Entity player = gm.createEntity();

    gm.addComponent<Player>(player);
    gm.addComponent<RType::Position2D>(player, 100.f, 300.f);
    gm.addComponent<RType::Velocity2D>(player);
    gm.addComponent<Hitbox>(player, 64.f, 32.f);
    gm.addComponent<RType::Texture2D>(player, ASSETS_DIR "player.png");
    gm.addComponent<RType::Sprite2D>(player);
    gm.addComponent<RType::Sound>(player, listener, ASSETS_DIR "shot.mp3");
    return player;
}

RType::Entity spawnBullet(RType::GameManager &gm, RType::Position2D pos)
{
    RType::Entity bullet = gm.createEntity();

    gm.addComponent<Bullet>(bullet);
    gm.addComponent<RType::Position2D>(bullet, pos);
    gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);
    gm.addComponent<Hitbox>(bullet, 16.f, 8.f);
    gm.addComponent<DestroyOffScreen>(bullet);
    gm.addComponent<RType::Texture2D>(bullet, ASSETS_DIR "bullet.png");
    gm.addComponent<RType::Sprite2D>(bullet);
    return bullet;
}
```

These functions can be called at startup as well as from a system, during the game: `createEntity` is always immediate, and the `addComponent` calls are applied at the end of the frame (see [the frame](ECS_GUIDE.en.md#5-the-frame-immediate-and-deferred-operations)).

### One function that registers the systems

Systems are free functions, declared in `Systems.hpp` and written in `Systems.cpp` (see [Writing systems in files](ECS_GUIDE.en.md#writing-systems-in-files)). A `registerSystems` function registers all of them, in one place, in execution order: that is the topic of the next section.

---

## 5. The order of systems

Systems run **in the order they were created**. That order decides what each system sees: a movement system placed before the keyboard is read uses the keys of the previous frame. Here is the recommended order:

```cpp
void registerSystems(RType::GameManager &gm)
{
    // 1. Start the frame: frame rate, then clear the screen
    gm.createSystem<RType::Window>(
        [](RType::Window &window) {
            window.setTargetFps();
            window.beginDrawing();
        },
        std::numeric_limits<int>::min());

    // 2. Input: read the keyboard and mouse
    gm.createSystem<RType::KeyboardManager>(RType::KeyboardManager::systemKeyboardManager);
    gm.createSystem<RType::MouseManager>(RType::MouseManager::systemMouseManager);

    // 3. Time: advance the game's clock
    gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);

    // 4. Game logic
    gm.createSystem<RType::KeyboardManager>(steerPlayer);
    gm.createSystem<RType::KeyboardManager>(shoot);
    gm.createSystem<RType::MasterClock>(spawnEnemies);
    gm.createSystem(collisions);
    gm.createSystem(destroyOffScreen);
    gm.createSystem<Score, RType::Text2D>(showScore);

    // 5. Physics: apply velocities
    gm.createSystem<RType::MasterClock>(RType::Velocity2D::systemMasterVelocity2D);

    // 6. Animation
    gm.createSystem<RType::MasterClock>(
        RType::SpriteSheet2D::systemMasterAnimateSpriteSheet2D);

    // 7. Rendering: background first, then sprites, sheets and texts on top
    gm.createSystem<Background, RType::Texture2D, RType::Position2D>(drawBackground);
    gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(
        RType::Sprite2D::systemDrawSprite2D);
    gm.createSystem<RType::SpriteSheet2D, RType::Texture2D, RType::Position2D>(
        RType::SpriteSheet2D::systemDrawSpriteSheet2D);
    gm.createSystem<RType::Text2D, RType::Font, RType::Position2D>(
        RType::Text2D::systemDrawText2D);

    // 8. Sound
    gm.createSystem<RType::Sound>(RType::Sound::systemSound);

    // 9. End the frame: show what was drawn
    gm.createSystem<RType::Window>(
        [](RType::Window &window) { window.endDrawing(); },
        std::numeric_limits<int>::max());
}
```

- **The engine's systems are registered like yours**: `gm.createSystem<Components...>(Class::systemXxx)`, with the components in the same order as the function's parameters. `const` parameters are accepted.
- **Every drawing happens between `beginDrawing()` and `endDrawing()`.** The second argument of `createSystem` is the system's layer: systems run by layer, then in creation order. `std::numeric_limits<int>::min()` puts the start of the frame before everything, and `std::numeric_limits<int>::max()` its end after everything, wherever the other systems are created.
- **The `Master…` systems take the `MasterClock`**: `Velocity2D::systemMasterVelocity2D` moves every entity that has a `Position2D` and a `Velocity2D`, and `SpriteSheet2D::systemMasterAnimateSpriteSheet2D` animates every `SpriteSheet2D`. They run once per frame, because there is a single `MasterClock`.
- **`KeyboardManager::systemKeyboardManager` and `MasterClock::systemMasterClock` must run exactly once per frame**, before any system that reads the keyboard or the time.
- **The drawing order follows the order of systems**: the background is drawn, then sprites, animated sheets, and texts on top. Within a single system, the order of entities is not guaranteed (see [A background](#a-background)).

---

## 6. Basic features

### Drawing an image

An image on screen is three components on the same entity: `Texture2D` (the image), `Sprite2D` (how to draw it) and `Position2D` (where).

```cpp
RType::Entity ship = gm.createEntity();
gm.addComponent<RType::Position2D>(ship, 100.f, 300.f);
gm.addComponent<RType::Texture2D>(ship, ASSETS_DIR "player.png");

RType::Sprite2D sprite;
sprite.scale = 2.f;
sprite.flip = true;      // horizontal mirror
gm.addComponent<RType::Sprite2D>(ship, sprite);
```

`Sprite2D` also holds `origin` (the pivot, see `Origin2D`), `rotation` in degrees and `color` (a tint, white by default). To change these values, the most readable way is to create the object, set its fields, then pass it to `addComponent`, as above.

### Animating a sprite sheet

A sprite sheet is an image split into a grid of tiles of the same size. `SpriteSheet2D` draws one tile, then moves to the next one every `frameDuration`:

```cpp
RType::Entity enemy = gm.createEntity();
gm.addComponent<RType::Position2D>(enemy, 1100.f, 300.f);
gm.addComponent<RType::Texture2D>(enemy, ASSETS_DIR "enemy.png");

RType::SpriteSheet2D sheet;
sheet.recNb = {1, 4};                               // 1 row, 4 columns
sheet.frameDuration = std::chrono::milliseconds(80);
gm.addComponent<RType::SpriteSheet2D>(enemy, sheet);
```

- **`recNb` is a `Size2D`, whose order is `{height, width}`**: `{1, 4}` means 1 row and 4 columns. Neither value may be 0.
- **`SpriteSheet2D::systemMasterAnimateSpriteSheet2D` animates it** with the game's clock. `animation = false` freezes the current tile, and `currentTile` selects the tile shown.

### Moving an entity

`Velocity2D` moves its entity's `Position2D`, in pixels per second:

```cpp
gm.addComponent<RType::Position2D>(bullet, 100.f, 300.f);
gm.addComponent<RType::Velocity2D>(bullet, 800.f, 0.f);   // 800 px/s to the right
```

- **`Velocity2D::systemMasterVelocity2D` moves it** with the game's clock ([section 5](#5-the-order-of-systems)).
- **The coordinate system is raylib's**: `(0, 0)` at the top-left, `x` to the right, `y` downward.
- **For gravity**, add `GravityField2D` and its system (before the `Velocity2D` one). Its fields are `x`, `y` (the direction, downward by default) and `g`, the strength **in pixels/s²**. The default value (`9.80665`) is therefore barely visible on screen: use a few hundred instead.

```cpp
gm.addComponent<RType::GravityField2D>(player, 0.f, 1.f, 900.f);   // downward, 900 px/s²

gm.createSystem<RType::MasterClock>(
    RType::GravityField2D::systemMasterGravityField2D);
```

- **To slow down or speed up some entities only**, give them their own `Clock` and use the systems that take it, instead of the `Master…` ones. `Clock::speed` multiplies the time the entity sees: `0` freezes it (movement and animation), `2` makes it go twice as fast. It is an integer: no slow motion.

```cpp
gm.addComponent<RType::Clock>(bullet);

gm.createSystem<RType::Clock>(RType::Clock::systemClock);
gm.createSystem<RType::Velocity2D, RType::Position2D, RType::Clock>(
    RType::Velocity2D::systemVelocity2D);
```

### Reading the keyboard and mouse

The `KeyboardManager` and `MouseManager` of the input entity (see [section 3](#3-the-main-window-input-loop)) are refreshed every frame by their system. They expose three lists:

| List | Contents |
|---|---|
| `keyPressed` / `buttonPressed` | The keys and buttons pressed **during this frame** (a single turn) |
| `keyDown` / `buttonDown` | The keys and buttons **held** |
| `keyReleased` / `buttonReleased` | The keys and buttons released **during this frame** |

The `MouseManager` also provides `getPos()` (the cursor position, as a `Position2D`), `getDelta()` (its movement since the previous frame) and `getWheel()` (the wheel).

A system reads them by asking for the manager: `gm.createSystem<RType::KeyboardManager>(steerPlayer)` calls `steerPlayer(gm, keyboard)` once per frame, since there is a single `KeyboardManager`. The system then goes through the entities it moves with a view ([Controlling the player](#controlling-the-player)).

Keys are `RType::Key::Value`s (`Key::Value::Space`, `Key::Value::Left`…). The default layout is AZERTY; for a QWERTY keyboard: `gm.addComponent<RType::KeyboardManager>(input, RType::Key::Layout::QWERTY)`.

**Rebinding keys** is done with a `Keybind`, on the input entity, with its system placed **right after** the keyboard and mouse ones:

```cpp
gm.addComponent<RType::Keybind>(input);
RType::Keybind &bind = gm.getComponent<RType::Keybind>(input);
bind._keyToKey[RType::Key::Value::Z] = RType::Key::Value::Up;                       // Z acts as ↑
bind._mouseButtonToKey[RType::MouseButton::Value::Left] = RType::Key::Value::Space; // click = shoot

gm.createSystem<RType::Keybind, RType::MouseManager, RType::KeyboardManager>(
    RType::Keybind::systemUpdateKeybind);
```

### Controlling the player

A system on the `KeyboardManager` turns the held keys into a velocity for every `Player`. The movement itself is done by the `Velocity2D` system.

```cpp
static bool contains(const std::vector<RType::Key::Value> &keys, RType::Key::Value key)
{
    return std::ranges::find(keys, key) != keys.end();
}

void steerPlayer(RType::GameManager &gm, const RType::KeyboardManager &keyboard)
{
    const auto &down = keyboard.keyDown;

    for (auto &[e, player, vel] : gm.view<Player, RType::Velocity2D>().getVector()) {
        vel = {0.f, 0.f};
        if (contains(down, RType::Key::Value::Left))
            vel.x -= player.speed;
        if (contains(down, RType::Key::Value::Right))
            vel.x += player.speed;
        if (contains(down, RType::Key::Value::Up))
            vel.y -= player.speed;
        if (contains(down, RType::Key::Value::Down))
            vel.y += player.speed;
    }
}

gm.createSystem<RType::KeyboardManager>(steerPlayer);
```

### Shooting: creating entities during the game

The same way, a system on the `KeyboardManager` creates a bullet and plays the player's sound when `Space` has just been pressed. It iterates over the players with `getVector()` (see [Iterating over a view in a system](ECS_GUIDE.en.md#iterating-over-a-view-in-a-system-getvector)):

```cpp
void shoot(RType::GameManager &gm, const RType::KeyboardManager &keyboard)
{
    if (!contains(keyboard.keyPressed, RType::Key::Value::Space))
        return;
    for (auto &[e, player, pos, sound] :
        gm.view<Player, RType::Position2D, RType::Sound>().getVector()) {
        spawnBullet(gm, {pos.x + 64.f, pos.y + 12.f});
        sound.state = RType::Sound::State::PLAY;
    }
}

gm.createSystem<RType::KeyboardManager>(shoot);
```

We use `keyPressed`, not `keyDown`: otherwise a bullet would be fired every frame while the key is held.

### Spawning enemies at regular intervals

A "spawner" entity holds a `Spawner`. A system on the `MasterClock` adds the frame's time to it, and creates an enemy when the delay is reached:

```cpp
void spawnEnemies(RType::GameManager &gm, const RType::MasterClock &clock)
{
    static std::mt19937 rng(std::random_device {}());
    std::uniform_real_distribution<float> y(0.f, 720.f - 48.f);

    for (auto &[e, spawner] : gm.view<Spawner>().getVector()) {
        spawner.elapsed += clock.getElapsed();
        if (spawner.elapsed < spawner.every)
            continue;
        spawner.elapsed = std::chrono::milliseconds(0);
        spawnEnemy(gm, {1280.f, y(rng)});
    }
}

// in main
RType::Entity spawner = gm.createEntity();
gm.addComponent<Spawner>(spawner);

// in registerSystems
gm.createSystem<RType::MasterClock>(spawnEnemies);
```

`spawnEnemy` follows the `spawnBullet` model, with `Enemy`, a `Velocity2D` to the left (`-150.f, 0.f`) and a `SpriteSheet2D`.

### Collisions

The engine does not provide collisions yet: they are written with a `Hitbox` component and two nested views. The list of enemies is built **only once**, before the loop over bullets:

```cpp
static bool overlap(const RType::Position2D &a, const Hitbox &ha,
    const RType::Position2D &b, const Hitbox &hb)
{
    return a.x < b.x + hb.width && b.x < a.x + ha.width &&
        a.y < b.y + hb.height && b.y < a.y + ha.height;
}

void collisions(RType::GameManager &gm)
{
    auto enemies = gm.view<Enemy, RType::Position2D, Hitbox>().getVector();

    for (auto &[bullet, b, bPos, bBox] :
        gm.view<Bullet, RType::Position2D, Hitbox>().getVector()) {
        for (auto &[enemy, en, ePos, eBox] : enemies) {
            if (en.hp <= 0 || !overlap(bPos, bBox, ePos, eBox))
                continue;
            gm.destroyEntity(bullet);
            if (--en.hp <= 0) {
                gm.destroyEntity(enemy);
                for (auto &[e, score] : gm.view<Score>().getVector())
                    score.value += 100;
            }
            break;   // a bullet hits only one enemy
        }
    }
}
```

The `en.hp <= 0` check matters: `destroyEntity` is deferred, so a killed enemy stays in the list until the end of the frame. Without that check, a second bullet could "kill" it again and count the points twice.

### Destroying what leaves the screen

The `DestroyOffScreen` tag marks the entities to remove when they leave the screen (bullets and enemies, not the player):

```cpp
void destroyOffScreen(RType::GameManager &gm)
{
    for (auto &[e, tag, pos] :
        gm.view<DestroyOffScreen, RType::Position2D>().getVector()) {
        if (pos.x < -100.f || pos.x > 1280.f + 100.f)
            gm.destroyEntity(e);
    }
}
```

Without this system, bullets would keep existing, moving and being drawn off screen, forever. For a fixed lifetime (explosions, particles), see the recipe [A limited lifetime](ECS_GUIDE.en.md#a-limited-lifetime-bullets-particles-explosions).

### Drawing text

A text is `Text2D` (the content and style), `Font` (the font) and `Position2D`:

```cpp
RType::Entity score = gm.createEntity();
gm.addComponent<Score>(score);
gm.addComponent<RType::Position2D>(score, 20.f, 20.f);
gm.addComponent<RType::Font>(score, ASSETS_DIR "font.ttf");

RType::Text2D text;
text.fontSize = 32.f;
text.str = "Score: 0";
gm.addComponent<RType::Text2D>(score, text);
```

`Text2D` also holds `color`, `rotation`, `spacing` and `origin`. For a changing text, a system updates `str` from the game data:

```cpp
void showScore(const Score &score, RType::Text2D &text)
{
    text.str = "Score: " + std::to_string(score.value);
}

gm.createSystem<Score, RType::Text2D>(showScore);
```

### Playing a sound

A `Sound` is created from the game's `Listener` (see [section 3](#3-the-main-window-input-loop)), then driven by its `state` field, which `Sound::systemSound` applies on its next run:

```cpp
gm.addComponent<RType::Sound>(player, listener, ASSETS_DIR "shot.mp3");   // loop = false by default

// in a system
sound.state = RType::Sound::State::PLAY;     // play from the start
sound.state = RType::Sound::State::PAUSED;   // pause
sound.state = RType::Sound::State::START;    // resume after a pause
sound.state = RType::Sound::State::STOP;     // stop
```

For background music, pass a `SoundInfo` with `loop = true` as the third argument, then set `state` to `PLAY` right after adding it:

```cpp
gm.addComponent<RType::Sound>(music, listener, ASSETS_DIR "music.mp3", RType::SoundInfo {.loop = true});
gm.getComponent<RType::Sound>(music).state = RType::Sound::State::PLAY;
```

`sound.setVolume(v)` and `sound.setPitch(p)` change the volume and the pitch (`1` by default).

### A background

Within a single system, the drawing order of entities is not guaranteed: a background drawn with `Sprite2D` may end up in front of the ship. The solution is to give it **its own drawing system**, registered before the others, and **no** `Sprite2D` component:

```cpp
struct Background {};

void drawBackground(const Background &, const RType::Texture2D &texture,
    const RType::Position2D &pos)
{
    RType::Sprite2D::systemDrawSprite2D(RType::Sprite2D {}, texture, pos);
}

// in registerSystems, before the other drawing systems
gm.createSystem<Background, RType::Texture2D, RType::Position2D>(drawBackground);
```

---

## 7. Do

- **Create the window first**, before any `Texture2D` or `Font`.
- **Create a single `MasterClock` and a single `Listener`** at startup, and create the sounds from that `Listener`.
- **Create every system at startup**, in a single function (`registerSystems`), in the order frame start → input → time → logic → physics → animation → rendering → sound → frame end ([section 5](#5-the-order-of-systems)).
- **Put the start and the end of the frame on the lowest and the highest layer** (`std::numeric_limits<int>::min()` and `max()`), so that every drawing happens between them.
- **Use the `Master…` systems** to move and animate with the game's clock, and a `Clock` only on the entities that must have their own time.
- **Write small components**, each with a single role, and **use tags** (empty structs) to sort entities. `Bullet` + `DestroyOffScreen` is better than a `Bullet` that does everything.
- **Create each kind of entity in a `spawnXxx` function**: the same code is used at startup and during the game.
- **Keep the game state in components** (score, lives, level): systems find it with a view, and it goes away with its entity.
- **Iterate over views with `getVector()` in frame systems**, and build a list used in a nested loop only once ([collisions](#collisions)).
- **Keep the `Entity` values you need later**, and check `gm.hasEntity(e)` before using one: the entity may have been destroyed in the meantime ([pitfall 4](ECS_GUIDE.en.md#pitfall-4-keeping-a-pointer-or-reference-to-another-entitys-component), [projectile recipe](ECS_GUIDE.en.md#a-projectile-aiming-at-a-target-that-may-be-destroyed)).
- **Call `Texture2D::unloadTextures2D(gm)` at the end of `main`**, before the window goes away.
- **Catch `RType::EngineException` in `main`** and return `84`.
- **Pass file paths through `ASSETS_DIR`** ([section 2](#the-cmakeliststxt)).
- **Test the logic without a window**: systems that only use game components and `engine-system` (collisions, score, destruction) can be tested with a plain `GameManager`, without raylib, for instance in the Criterion tests.
- **Run `./script/clang.sh` and `./script/lint.sh` before pushing.**

---

## 8. Don't

- **Don't use `EntityManager`, `ComponentManager` or `SystemManager`**: everything goes through the `GameManager`.
- **Don't load a texture or a font before the window**: loading fails or crashes.
- **Don't create two `Window`s or two `Listener`s**: the second constructor throws an `EngineException`. Likewise, **a single `KeyboardManager`, a single `MouseManager` and a single `MasterClock`** for the whole game.
- **Don't destroy the `Listener` before the sounds**: every `Sound` uses it until it is destroyed.
- **Don't draw outside `beginDrawing()` and `endDrawing()`**: keep the start and the end of the frame on the extreme layers ([section 5](#5-the-order-of-systems)).
- **Don't create a system during `update()`** ([pitfall 2](ECS_GUIDE.en.md#pitfall-2-creating-a-system-during-update)): every system at startup. To disable a behavior, remove a component or use a tag ([recipe](ECS_GUIDE.en.md#pausing-an-entity-without-destroying-a-system)).
- **Don't call `gm.update()` from a system**: the call does nothing.
- **Don't read a component right after adding it inside a system**: it only exists at the end of the frame ([pitfall 7](ECS_GUIDE.en.md#pitfall-7-reading-a-component-right-after-adding-it-inside-a-system)).
- **Don't put logic or calls to the `GameManager` in a component**: logic goes in systems ([pitfall 1](ECS_GUIDE.en.md#pitfall-1-calling-the-gamemanager-from-a-components-constructor-or-destructor)).
- **Don't swap `Size2D`**: it is `{height, width}`, not `{width, height}`.
- **Don't register a `Master…` system and the matching system with a `Clock` for the same entities**: they would be moved twice per frame.
- **Don't rely on the order of entities in a system**: for a drawing order, use separate systems ([background](#a-background)).
- **Don't leave useless entities alive**: off-screen bullets, finished explosions… Every forgotten entity keeps being updated and drawn.
- **Don't change the engine for a game-specific need**: write a component and a system on the game side. If the engine needs to change, discuss it with the engine team.
- **Don't run `cmake .` at the root**: always use `-B build` (see the [README](../README.md)).

---

## 9. Current engine limitations

What the engine does not do yet, and what to do in the meantime:

| Limitation | In the meantime |
|---|---|
| No drawing order (depth) within a system | One drawing system per "layer" ([background](#a-background)) |
| No collisions | A `Hitbox` component and a game-side system ([collisions](#collisions)) |
| `Clock::speed` is an integer | No slow motion; `0` to freeze, `1` for normal speed |

---

## 10. Going further

| Document | Contents |
|---|---|
| [README](../README.md) | Installation, build, tests, linter |
| [ECS user guide](ECS_GUIDE.en.md) | The whole `GameManager` API, in detail |
| ECS guide, [Systems](ECS_GUIDE.en.md#4-systems) | Per-entity systems, frame systems, with the `GameManager`, `getVector()` |
| ECS guide, [The frame](ECS_GUIDE.en.md#5-the-frame-immediate-and-deferred-operations) | What is immediate and what is deferred during `update()` |
| ECS guide, [Pitfalls to avoid](ECS_GUIDE.en.md#7-pitfalls-to-avoid) | The most common mistakes, with their explanation |
| ECS guide, [Recipes](ECS_GUIDE.en.md#8-recipes) | Lifetime, pause, homing projectile, collisions |
| ECS guide, [API cheat sheet](ECS_GUIDE.en.md#9-api-cheat-sheet) | Every method on one page |
| [ECS design notes](ECS_DESIGN.en.md) | The why behind the engine's architecture |
| [Accessibility](ACCESSIBILITY.en.md) | What the engine offers for accessibility, why it helps and who for |
| [Comparative studies](comparative-studies/en/README.md) | The project's technical choices and their alternatives: language, build, ECS, raylib, networking, protocol, tests… |
| `Engine/*/include/` | The Doxygen documentation of each provided component |
| `tests/engine/` | The ECS unit tests: a usage example for every behavior |
