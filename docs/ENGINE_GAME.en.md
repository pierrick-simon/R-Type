# Running a game: `game`

> 🌐 **Language:** [Français](ENGINE_GAME.fr.md) · English

`game` is the engine's executable. A game has no C++ and is not compiled: it is a folder that holds two things:

- **a `config.conf` file**: the window, the frame rate, the scripts folder and the engine modules to load;
- **Lua scripts**: they create the entities, give them components, register the systems and decide when the game stops.

The engine does the rest: it creates the `GameManager`, loads the modules, runs every script, then runs the loop.

- **Audience:** anyone making a game with the engine.
- **To write the scripts**, see [the Lua scripting guide](LUA_GUIDE.en.md).
- **To write them faster in VS Code**, install the **R-Type lua engine epitech** extension, published by Ariel Amriou (publisher ID `ariel-amriou`):

  ```sh
  code --install-extension ariel-amriou.r-type-lua-engine-epitech
  ```
- **For the ECS in detail** (entities, components, systems, views), see [the ECS user guide](ECS_GUIDE.en.md).

> [!TIP]
> Every example in this guide was run against the current engine, and every error message in [section 8](#8-errors-and-how-to-fix-them) was reproduced.

---

## Contents

1. [What the engine does at start](#1-what-the-engine-does-at-start)
2. [The `config.conf` file](#2-the-configconf-file)
3. [The modules](#3-the-modules)
4. [Setting up the game](#4-setting-up-the-game)
5. [A complete game](#5-a-complete-game)
6. [Debug mode](#6-debug-mode)
7. [Do / don't](#7-do--dont)
8. [Errors and how to fix them](#8-errors-and-how-to-fix-them)

---

## 1. What the engine does at start

The `main` lives in [`Engine/Game/src/Main.cpp`](../Engine/Game/src/Main.cpp) and calls `RType::Game::Runner::run()`. In order, it:

1. reads the `config.conf` file, which must exist (see [section 2](#2-the-configconf-file));
2. creates the `GameManager` (in debug mode if the engine is built with `__DEBUG__`, see [section 6](#6-debug-mode));
3. loads the modules listed in `modules`, plus `Clock`, which is always loaded (see [section 3](#3-the-modules));
4. loads every `.lua` file of the scripts folder (`srcDirectory`) and its subfolders, **in alphabetical order of their path**, and calls the `start(gm)` function of each one (see [the Lua guide](LUA_GUIDE.en.md));
5. runs the loop, at `windowFPS` frames per second, **until a script calls `gm:stop()`** (see [the Lua guide](LUA_GUIDE.en.md#stopping-the-game));
6. if an `RType::EngineException` comes up, from the engine or from a script, prints its message and exits with code `84`.

- **Only a script stops the game.** Closing the window is not enough: a game that does not check `gm:windowShouldClose()` to call `gm:stop()` keeps running after a click on the close button.
- **Everything is looked for relative to the directory the game is started from**: `config.conf`, the scripts folder, and the paths the scripts give (images, fonts, sounds…).

---

## 2. The `config.conf` file

```
# window
windowFPS = 60
windowWidth = 1280
windowHeight = 720
windowName = Bounce

srcDirectory = scripts
modules = Window, Keyboard, Mouse, Audio
```

| Key | Role | Default |
|---|---|---|
| `windowFPS` | Frames per second of the loop | `60` |
| `windowWidth`, `windowHeight` | Window size, in pixels, used by the `Window` module | `800` × `600` |
| `windowName` | Window title, used by the `Window` module | `My Game` |
| `srcDirectory` | The folder of the Lua scripts, relative to the directory the game is started from | `.` (the start directory itself) |
| `modules` | The engine modules to load, separated by commas (see [section 3](#3-the-modules)) | none (only `Clock`) |

- **The file is required**, even empty: without it, the game stops at start.
- **Every key is optional.** A key left out keeps its default. If a key appears twice, the last one wins.
- **One line per setting**, as `key = value`. The spaces around the key, the `=` and the value are ignored. Keys are case sensitive.
- **Empty lines and lines starting with `#` are ignored.**
- **`windowFPS`, `windowWidth` and `windowHeight`** are whole numbers greater than 0.
- **An invalid line stops the game** before anything runs, with the file and the line (see [section 8](#8-errors-and-how-to-fix-them)).

For engine developers: the file fills the `RType::GameConfig` variable, of type `GameInfo`, declared in [`Game.hpp`](../Engine/Game/include/Game.hpp) and read by [`ConfigBuilder`](../Engine/Game/include/ConfigBuilder.hpp). Adding a key is explained in [the engine extension guide](ENGINE_EXTENDING.en.md#6-summary).

---

## 3. The modules

A module is an engine feature the game turns on by its name. Each one creates its entity and registers its system, **before** the scripts: the scripts' systems therefore always run after them.

| Module | What it creates | Needs |
|---|---|---|
| `Window` | an entity with a `Window` (size and title from `config.conf`), and the system that clears and refreshes it each frame; scripts know whether it should close with `gm:windowShouldClose()` | |
| `Keyboard` | an entity with a `KeyboardManager`, and the system that reads the keys; scripts read it with `gm:keyboard()` | `Window` |
| `Mouse` | an entity with a `MouseManager`, and the system that reads the mouse; scripts read it with `gm:mouse()` | `Window` |
| `Audio` | an entity with the point that hears the sounds, and the system that plays the `Sound`s | |
| `Clock` | an entity with the `MasterClock`, and the system that moves it forward; scripts read its time with `gm:elapsed()` | always loaded |

- **`Clock` is always loaded**, even if `modules` does not list it: the engine's movement and animation systems rely on it.
- **A module is loaded only once**, even if it is listed twice.
- **The order is kept**: the game's modules first, in its order, then `Clock`.
- **A missing dependency stops the game** before anything runs, for instance `Keyboard` without `Window` (see [section 8](#8-errors-and-how-to-fix-them)).

---

## 4. Setting up the game

### The folder

```
Games/
├── game                     # the engine's executable
└── MyGame/
    ├── config.conf          # srcDirectory = scripts
    ├── scripts/
    │   └── game.lua         # the scripts, loaded in alphabetical order
    └── sprites/
        └── ship.png
```

No `CMakeLists.txt`, no `.cpp`: the game is not compiled.

### Building the engine

```sh
cmake -S . -B build
cmake --build build --target game -j
```

The executable is placed in `Games/`. It is built once, for every game: changing a script or `config.conf` needs no recompiling.

### Running the game

```sh
cd Games/MyGame && ../game
```

The game must be **started from its folder**, the one that holds `config.conf`.

---

## 5. A complete game

A ship crossing the window and bouncing off its edges. `Escape` or the close button quit the game.

```
# config.conf
windowName = Bounce
srcDirectory = scripts
modules = Window
```

```lua
-- scripts/game.lua
local WIDTH, HEIGHT = 800, 600

local function bounce(gm, e, pos, vel)
    if pos.x < 0 or pos.x > WIDTH - 32 then vel.x = -vel.x end
    if pos.y < 0 or pos.y > HEIGHT - 32 then vel.y = -vel.y end
end

local function quit(gm)
    if gm:windowShouldClose() then gm:stop() end
end

function start(gm)
    local ship = gm:createEntity()
    gm:addComponent(ship, 'Position2D', {x = 100, y = 100})
    gm:addComponent(ship, 'Velocity2D', {x = 250, y = 180})
    gm:addComponent(ship, 'Texture2D', {path = 'sprites/ship.png'})
    gm:addComponent(ship, 'Sprite2D', {scale = 2})

    gm:createSystem(quit)
    gm:createSystem({'Position2D', 'Velocity2D'}, bounce)
    gm:addSystem('MasterVelocity2D')
    gm:addSystem('DrawSprite2D')
end
```

- **`config.conf`** only turns on the window: the default size, 800 × 600, is fine. The `Window` module already clears the window each frame.
- **`start`** runs once: it creates the ship, then registers four systems, which run each frame in this order. `quit` and `bounce` are the game's own: `quit` stops the game with `gm:stop()` when the window should close; `MasterVelocity2D` (movement) and `DrawSprite2D` (drawing) come from the engine.

Everything a script can do is in [the Lua scripting guide](LUA_GUIDE.en.md).

---

## 6. Debug mode

When the engine is built with the `__DEBUG__` macro, it creates the `GameManager` in debug mode: every operation is written to `std::clog`.

```sh
cmake -S . -B build -DCMAKE_CXX_FLAGS="-D__DEBUG__"
cmake --build build --target game -j
```

```
[GameManager] debug mode enabled
[GameManager] createEntity -> entity 0
[GameManager] addComponent<RType::MasterClock> on entity 0
[GameManager] createSystem with ( RType::MasterClock ) -> system 0
```

---

## 7. Do / don't

### Do

- **List `Window` in `modules`** as soon as the game draws something, reads the keyboard or the mouse, or calls `gm:windowShouldClose()`.
- **Have a system that calls `gm:stop()`** when `gm:windowShouldClose()` returns `true`: without it, the window does not close.
- **Start the game from its folder**, the one that holds `config.conf`.
- **Write the scripts' paths relative to that folder.**

### Don't

- **Don't list `Clock` in `modules`**: it is always loaded.
- **Don't start the game from another folder**: the engine finds neither `config.conf` nor the scripts there.

---

## 8. Errors and how to fix them

Each error stops the game with exit code `84`, before anything runs.

### `Engine Error: Config error: ...`

```
Engine Error: Config error: config.conf:2: unknown key 'fps'
Engine Error: Config error: config.conf:1: windowWidth expects a number greater than 0, got 'abc'
Engine Error: Config error: config.conf:4: expected 'key = value'
Engine Error: Config error: config.conf: cannot be opened
```

**Cause:** a line of `config.conf` has a key that does not exist, an invalid value, or no `=`, and the message gives the line. Or the file is not in the directory the game is started from.
**Fix:** correct the line, with the keys and values of [section 2](#2-the-configconf-file). Or start the game from its folder, or create `config.conf` there, even empty.

### `Engine Error: No such module: ...`

```
Engine Error: No such module: Windw
```

**Cause:** a name in `modules` is not one of the modules of [section 3](#3-the-modules), often because of a typo.
**Fix:** correct the name. The module names are case sensitive.

### `Engine Error: Module ... requires module ...`

```
Engine Error: Module Keyboard requires module Window
```

**Cause:** a module is listed without the module it needs.
**Fix:** add the missing module to `modules`.

### `Engine Error: Script error: ...`

**Cause:** a script has a mistake, or calls the engine with wrong arguments. The message gives the file and the line.
**Fix:** see [the errors of the Lua guide](LUA_GUIDE.en.md#9-errors-and-how-to-fix-them).

### Nothing happens: no script is run

**Cause:** `srcDirectory` does not point to the scripts folder. The engine finds no script, and runs an empty loop without error.
**Fix:** check `srcDirectory` in `config.conf`, written relative to the directory the game is started from.

### The window does not open

**Cause:** `Window` is not listed in `modules`.
**Fix:** add `Window` to `modules`.

### The window does not close

**Cause:** no script calls `gm:stop()`: the close button and `Escape` do nothing but make `gm:windowShouldClose()` true.
**Fix:** add the `quit` system of [the complete game](#5-a-complete-game).
