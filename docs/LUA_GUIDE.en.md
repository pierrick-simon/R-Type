# Scripting a game in Lua

> 🌐 **Language:** [Français](LUA_GUIDE.fr.md) · English

With `engine-game`, a game's content is written in Lua: the scripts create the entities, give them the engine's components, and register the systems that run each frame. The C++ side of the game only holds its settings (`RType::GameConfig`).

- **Audience:** anyone writing a game's scripts, and, in [section 10](#10-for-engine-developers-exposing-a-component-or-a-system), engine developers who make a new component or system usable from Lua.
- **To set up the game** (`GameConfig`, modules, folder, CMake), see [the engine-game guide](ENGINE_GAME.en.md).

> [!TIP]
> Every example in this guide was run against the current engine, and every error message in [section 9](#9-errors-and-how-to-fix-them) was reproduced.

---

## Contents

1. [A script](#1-a-script)
2. [Entities](#2-entities)
3. [Components](#3-components)
4. [Systems](#4-systems)
5. [Views: looping over entities anywhere](#5-views-looping-over-entities-anywhere)
6. [What happens during a frame](#6-what-happens-during-a-frame)
7. [A complete game](#7-a-complete-game)
8. [Do / don't](#8-do--dont)
9. [Errors and how to fix them](#9-errors-and-how-to-fix-them)
10. [For engine developers: exposing a component or a system](#10-for-engine-developers-exposing-a-component-or-a-system)
11. [API cheat sheet](#11-api-cheat-sheet)

---

## 1. A script

The engine loads every `.lua` file of the `TestLua/` folder and its subfolders, in alphabetical order of their path, then calls the `start` function of each one, **once**, with the `GameManager`:

```lua
function start(gm)
    print('the game starts')
end
```

- **Every script must define `start`.** A script without it stops the game (see [section 9](#9-errors-and-how-to-fix-them)).
- **`start` creates things; systems run them.** What must happen every frame goes into a system, registered from `start` ([section 4](#4-systems)).
- **Each script has its own globals.** A global variable or function defined in `a.lua` is not visible from `b.lua`, so two scripts can both name their function `process` without conflict. Use `local` for everything else anyway.
- **The Lua base library is available**: `print`, `pairs`, `ipairs`, `pcall`, `error`, `type`, `tostring`, `tonumber`…
- **Paths** (images, fonts, models) are relative to the folder the game is started from, the one that holds `TestLua/`.

---

## 2. Entities

```lua
local e = gm:createEntity()     -- a new entity
gm:hasEntity(e)                 -- true
gm:destroyEntity(e)             -- destroys it, with all its components
gm:hasEntity(e)                 -- false
print(e)                        -- #1 (0): its index, then its generation
```

An entity is only a handle. Entities compare with `==`. A destroyed entity's handle is never confused with a new entity that reuses its index.

---

## 3. Components

### Adding a component

```lua
gm:addComponent(e, 'Position2D', {x = 100, y = 50})
gm:addComponent(e, 'Velocity2D', {x = 250})     -- y keeps its default, 0
gm:addComponent(e, 'Clock')                     -- every field keeps its default
```

The table is optional. **Fields left out keep their default value.** A field that does not exist, or a value of the wrong type, stops with a clear message:

```
Invalid arguments for Position2D: no field 'z'
Invalid arguments for Position2D: field 'x' expects a number, got a string
```

### Reading and changing a component

```lua
gm:hasComponent(e, 'Position2D')            -- true / false, never an error
local pos = gm:getComponent(e, 'Position2D')
pos.x = 42                                  -- changes the real component
gm:removeComponent(e, 'Position2D')
gm:countComponents('Position2D')            -- number of entities that have one
```

`getComponent` gives **the real component, not a copy**: writing its fields changes it for the whole engine, including the C++ systems.

### The components

| Name | Fields (default) | Role |
|---|---|---|
| `Position2D` | `x` (0), `y` (0) | position in pixels, (0, 0) at the top-left corner |
| `Velocity2D` | `x` (0), `y` (0) | speed in pixels per second |
| `Origin2D` | `x` (0), `y` (0) | origin point |
| `Size2D` | `width` (0), `height` (0) | size in pixels |
| `Friction2D` | `f` (0) | slows a `Velocity2D` down |
| `GravityField2D` | `x` (0), `y` (1), `g` (9.80665) | pulls a `Velocity2D` in a direction |
| `Clock` | `speed` (1) | an entity's own clock |
| `Texture2D` | `path` (**required**) | an image, loaded from `path` |
| `Sprite2D` | `scale` (1), `rotation` (0), `flip` (false) | draws the entity's `Texture2D` |
| `SpriteSheet2D` | `rows` (1), `columns` (1), `currentTile` (0), `frameDuration` (100, in ms), `animation` (true), `scale` (1), `rotation` (0), `flip` (false) | draws and animates one tile of the `Texture2D` at a time |
| `Font` | `path` (**required**) | a font, loaded from `path` |
| `Text2D` | `str` ("Hello World"), `fontSize` (10), `rotation` (0), `spacing` (0) | draws a text with the entity's `Font` |
| `Position3D` | `x` (0), `y` (0), `z` (0) | position in 3D space |
| `Model3D` | `path` (**required**) | a 3D model, loaded from `path` |
| `ModelAnimation3D` | `path` (**required**) | the animations of a 3D model |
| `Sprite3D` | `scale` (1) | draws the entity's `Model3D` |
| `SpriteSheet3D` | `index` (0), `frameDuration` (100, in ms), `animation` (true), `scale` (1) | plays an animation of the `Model3D` |
| `Listener` | | the point that hears the 2D sounds |

The four components marked **required** are built from a file: `gm:addComponent(e, 'Texture2D', {path = 'TestLua/sprites/ship.png'})`.

---

## 4. Systems

A system is a function the engine calls **every frame**. Systems run **in the order they are created**: the modules' systems first (window, keyboard, clock), then the scripts', in the order of the scripts and of the calls.

### The game's own systems

**Once per frame:**

```lua
local frame = 0

function process(gm)
    frame = frame + 1
end

function start(gm)
    gm:createSystem(process)
end
```

**Once per entity that has some components:**

```lua
function process(gm, e, pos, vel)
    if pos.x > 800 then vel.x = -vel.x end
end

function start(gm)
    gm:createSystem({'Position2D', 'Velocity2D'}, process)
end
```

- The function receives the `GameManager`, the entity, then **one component per name, in the list's order**. `{'Velocity2D', 'Position2D'}` gives `process(gm, e, vel, pos)`.
- Only the entities that have **all** the components are given. If one of them has never been added, the function is simply not called.
- The components are the real ones: writing their fields changes them.
- The system can be created **before** the entities: it finds them as soon as they have the components.
- A wrong name stops the game **at `start`**, not at the first frame.
- An empty list, `{}`, is the same as no list: once per frame.

### The engine's systems

`gm:addSystem(name)` adds a system written in C++ by the engine. It acts on the entities that have the components listed in the table.

| Name | Acts on the entities that have | Role |
|---|---|---|
| `MasterVelocity2D` | `Position2D`, `Velocity2D` | moves them, with the game's clock |
| `Velocity2D` | `Velocity2D`, `Position2D`, `Clock` | moves them, with their own clock |
| `MasterFriction2D` | `Velocity2D`, `Friction2D` | slows them down, with the game's clock |
| `Friction2D` | `Friction2D`, `Velocity2D`, `Clock` | slows them down, with their own clock |
| `MasterGravityField2D` | `Velocity2D`, `GravityField2D` | applies the gravity, with the game's clock |
| `GravityField2D` | `GravityField2D`, `Velocity2D`, `Clock` | applies the gravity, with their own clock |
| `Clock` | `Clock` | advances their own clock |
| `DrawSprite2D` | `Sprite2D`, `Texture2D`, `Position2D` | draws them |
| `DrawSpriteSheet2D` | `SpriteSheet2D`, `Texture2D`, `Position2D` | draws their current tile |
| `MasterAnimateSpriteSheet2D` | `SpriteSheet2D` | animates them, with the game's clock |
| `AnimateSpriteSheet2D` | `SpriteSheet2D`, `Clock` | animates them, with their own clock |
| `DrawText2D` | `Text2D`, `Font`, `Position2D` | draws their text |
| `DrawSprite3D` | `Sprite3D`, `Model3D`, `Position3D` | draws them in 3D |
| `DrawSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `Position3D` | draws them in 3D, animated |
| `MasterAnimateSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `ModelAnimation3D` | animates them, with the game's clock |
| `AnimateSpriteSheet3D` | `SpriteSheet3D`, `Model3D`, `ModelAnimation3D`, `Clock` | animates them, with their own clock |
| `Listener2D` | `Listener`, `Position2D` | moves the point that hears the sounds |
| `KeyboardManagerClear` | the keyboard | empties the keys read this frame |

The `Master…` systems use the game's clock, which the engine always runs: they are the simplest choice. The others use a `Clock` given to each entity, to slow down or speed up some entities only (with its `speed`), and need the `Clock` system too.

The systems of the window, the keyboard, the mouse and the game's clock are registered by the [modules](ENGINE_GAME.en.md#3-the-modules), once: they are not in this table, so that no script makes them run twice per frame. The other engine systems (`Draw3D`, `UpdateCamera`, `UpdateKeybind`, `Sound`, `Sound2D`) act on components that are created from C++.

### The order of the systems

The usual order, inside `start`:

1. the game's logic (reading the keyboard, collisions, rules);
2. the movement (`MasterVelocity2D`…);
3. the drawing (`DrawSprite2D`, `DrawText2D`…). The window is already cleared by the `Window` module, whose system runs first.

### Stopping a system

`gm:createSystem` and `gm:addSystem` return the id of the created system. `gm:destroySystem(id)` stops it, and `gm:hasSystem(id)` tells whether it still exists:

```lua
local intro
local frame = 0

function showIntro(gm)
    frame = frame + 1
    if frame == 180 then
        gm:destroySystem(intro)       -- the intro stops after 180 frames
    end
end

function start(gm)
    intro = gm:createSystem(showIntro)
end
```

- A system can stop itself, or another one, from a system: it still runs until the end of the frame ([section 6](#6-what-happens-during-a-frame)).
- Stopping a system that is already stopped does nothing.

---

## 5. Views: looping over entities anywhere

`gm:view` loops over the entities that have some components, **from `start` or from inside a system**:

```lua
for e, pos, vel in gm:view({'Position2D', 'Velocity2D'}) do
    pos.x = pos.x + vel.x
end
```

- Same rules as a system: the entity, then one component per name in the list's order, the real components, a wrong name stops with an error.
- `break` and `return` work as in any loop.
- **The loop can destroy entities**, even in `start`: an entity destroyed or changed during the loop, before its turn, is skipped.
- Inside a system, it crosses two groups of entities, for instance everything that moves against every obstacle:

```lua
function process(gm)
    for mover, mpos, vel in gm:view({'Position2D', 'Velocity2D'}) do
        for obstacle, opos, size in gm:view({'Position2D', 'Size2D'}) do
            -- test the collision between mover and obstacle
        end
    end
end
```

---

## 6. What happens during a frame

During a frame, while the systems run, the changes a script asks for are **applied at the end of the frame**, all together:

- `gm:destroyEntity`, `gm:addComponent`, `gm:removeComponent` and `gm:destroySystem` are applied at the end of the frame;
- `gm:createEntity` is immediate.

This is what lets a system destroy entities while the engine is going through them. It has two visible effects, inside the same frame:

```lua
function process(gm, e, f)
    gm:destroyEntity(e)
    gm:hasEntity(e)                   -- still true until the end of the frame
end
```

```lua
function process(gm, e, pos)
    gm:addComponent(e, 'Velocity2D')
    gm:hasComponent(e, 'Velocity2D')  -- still false until the end of the frame
end
```

In `start`, outside the frame, every change is immediate.

A change that cannot be applied at the end of the frame stops the game then, for instance a component added to an entity destroyed in the same frame (see [section 9](#9-errors-and-how-to-fix-them)).

**Systems are created only in `start`.** Calling `gm:createSystem` or `gm:addSystem` from a system stops the game with an error: the new system would run in the current frame, and a system created for each entity at each frame would make the systems pile up until the game slows down.

---

## 7. A complete game

A ship crossing the window and bouncing off its edges. The setup (`Setup.cpp` with the `Window` and `Keyboard` modules) is in [the engine-game guide](ENGINE_GAME.en.md#5-a-complete-game).

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

Each frame: `bounce` turns the ship around at the edges, `MasterVelocity2D` moves it, `DrawSprite2D` draws it.

---

## 8. Do / don't

### Do

- **Define `start` in every script.**
- **Create the entities and the systems in `start`**, and put what runs every frame in systems.
- **Give a system the list of components it works on**, rather than looping with a view inside a once-per-frame system: the engine then picks the entities for you.
- **Prefer the `Master…` engine systems**, unless some entities need their own clock.
- **Use `local`** for the script's variables and helper functions.
- **Write the paths relative to the folder that holds `TestLua/`.**

### Don't

- **Don't call `gm:createSystem` or `gm:addSystem` from a system** ([section 6](#6-what-happens-during-a-frame)).
- **Don't expect a change made in a system to be visible in the same frame** ([section 6](#6-what-happens-during-a-frame)).
- **Don't give an entity the same component twice**: remove it first, or change its fields with `getComponent`.

---

## 9. Errors and how to fix them

Every error stops the game with exit code `84` and a message `Engine Error: Script error: ...`. The `stack traceback` lines under the message give the file and the line where it happened.

| Message | Cause | Fix |
|---|---|---|
| `attempt to call a nil value` | a script has no `start` function | define `function start(gm) ... end` |
| `TestLua/a.lua:1: ')' expected near 'end'` | a Lua syntax error, at this file and line | fix the syntax |
| `Unknown component: Postion2D` | the name is not one of [the components](#the-components) | correct the name (case sensitive) |
| `Unknown system: DrawSprit2D` | the name is not one of [the engine's systems](#the-engines-systems) | correct the name |
| `gm:createSystem cannot be called from a system, only from start` | `gm:createSystem` or `gm:addSystem` called from a system | create the system in `start`, and stop it or keep it idle with a variable of the script |
| `Invalid arguments for Position2D: no field 'z'` | the table gives a field the component does not have | remove or rename the field |
| `Invalid arguments for Position2D: field 'x' expects a number, got a string` | a field of the wrong type | give a value of the right type |
| `Invalid arguments for Texture2D: missing 'path'` | a component built from a file without `path` | give `{path = '...'}` |
| `Invalid component list: element 2 is not a name` | the list of a system or a view holds something else than a string | only put names in the list |
| `Invalid component list: Position2D is in it twice` | a name is twice in the list | remove the duplicate |
| `Already an existing component linked to this entity` | the entity already has this component | change it with `getComponent`, or remove it first |
| `No such component.` | `getComponent` or `removeComponent` on a component the entity does not have | check with `hasComponent` first |
| `sol: no matching function call takes this number of arguments and the specified types` | wrong arguments to an engine function, for instance `gm:createSystem('Position2D', f)` instead of `gm:createSystem({'Position2D'}, f)` | check the arguments against the [cheat sheet](#11-api-cheat-sheet) |
| `sol: cannot set (new_index) into this object: no defined new_index operation on usertype` | a system writes a field the component does not have, for instance `pos.z = 1` on a `Position2D` | use a field of the [table](#the-components) |
| `attempt to index a nil value (global 'math')` | `math`, `string`, `table`, `os` and `io` are not available | write the computation with the base library |

### `Engine Error: No such entity.` or `No such component.`, without `Script error`

These come **at the end of a frame**, without a file or a line: a change asked for by a system could not be applied once the frame ended ([section 6](#6-what-happens-during-a-frame)). For instance:

- `gm:addComponent` or `gm:removeComponent` on an entity that a system destroyed in the same frame;
- `gm:removeComponent` twice on the same component in the same frame.

**Fix:** check `gm:hasEntity` / `gm:hasComponent` before, or make sure only one system destroys or removes.

---

## 10. For engine developers: exposing a component or a system

Everything that makes the engine usable from Lua is listed in one file, [`Engine/Game/src/EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp).

### A component

Add one `bindComponent` line to `ComponentBinding::bindEngineComponents`: the C++ type, the name scripts use, then each field as `"name", &Type::member`.

```cpp
bindComponent<Position2D>(lua, "Position2D",
    "x", &Position2D::x, "y", &Position2D::y);
```

This single line gives the component everything of [section 3](#3-components) and of the systems and views: `addComponent` with a table and defaults, the field checks and their messages, `getComponent`, `hasComponent`, `removeComponent`, `countComponents`, and its use in `createSystem` and `gm:view`. Changing a field means changing only this line.

- **A component without a default constructor**, such as `Texture2D`, is built from the script's `path` field. Give it no field: `bindComponent<Texture2D>(lua, "Texture2D");`.
- **A field of a field** is exposed with `nested`. `SpriteSheet2D::recNb.height` becomes `rows`:

  ```cpp
  "rows", nested(&SpriteSheet2D::recNb, &Size2D::height),
  ```

- **A `std::chrono::milliseconds` member** is exposed as a number of milliseconds with `milliseconds`:

  ```cpp
  "frameDuration", milliseconds(&SpriteSheet2D::frameDuration),
  ```

### A system

Add one line to `SystemBinding::engineSystems`: the name scripts use, which is the C++ function's without its `system` prefix, then `makeSystem` with the function.

```cpp
{"DrawSprite2D", makeSystem(Sprite2D::systemDrawSprite2D)},
```

`makeSystem` reads the components from the function's own parameters, with or without a `GameManager &` first: there is nothing else to write.

---

## 11. API cheat sheet

| Call | Does |
|---|---|
| `gm:createEntity()` | creates an entity and returns it |
| `gm:destroyEntity(e)` | destroys an entity and its components |
| `gm:hasEntity(e)` | whether the entity exists |
| `gm:addComponent(e, name [, fields])` | adds a component, built from the table |
| `gm:getComponent(e, name)` | the real component |
| `gm:hasComponent(e, name)` | whether the entity has the component |
| `gm:removeComponent(e, name)` | removes the component |
| `gm:countComponents(name)` | how many entities have the component |
| `gm:createSystem(func)` | calls `func(gm)` every frame |
| `gm:createSystem({names}, func)` | calls `func(gm, e, components...)` every frame, for every entity that has them |
| `gm:addSystem(name)` | adds one of the engine's systems |
| `gm:destroySystem(id)` | stops a system, with the id returned by `createSystem` or `addSystem` |
| `gm:hasSystem(id)` | whether the system exists |
| `for e, components... in gm:view({names}) do` | loops over the entities that have the components |
