# Extending the engine: components, systems, modules

> 🌐 **Language:** [Français](ENGINE_EXTENDING.fr.md) · English

This guide explains how to add a component, a system or a module to the engine, and how to make them usable from Lua scripts. Each addition is made in C++, in one precise place, and the rest of the engine (field checks, error messages, Lua systems and views) uses it with no other code.

- **Audience:** the engine's developers.
- **To write a component or a system in C++ inside a game**, see [the ECS guide](ECS_GUIDE.en.md). **To use them from Lua**, see [the Lua guide](LUA_GUIDE.en.md).

---

## Contents

1. [Where the code is](#1-where-the-code-is)
2. [Adding a component](#2-adding-a-component)
3. [Adding a system](#3-adding-a-system)
4. [Adding a module](#4-adding-a-module)
5. [Adding a Lua function to the `GameManager`](#5-adding-a-lua-function-to-the-gamemanager)
6. [Summary](#6-summary)

---

## 1. Where the code is

| Library | Folder | Contents |
|---|---|---|
| `engine-system` | [`Engine/System`](../Engine/System) | the ECS, and the basic components (`Position2D`, `Position3D`, `Clock`…) in `include/BuiltIn` |
| `engine-physics` | [`Engine/Physics`](../Engine/Physics) | movement, friction, gravity and rotation, in 2D and 3D (`Velocity2D`, `Rotation3D`…) |
| `engine-graphical2D` | [`Engine/Graphical2D`](../Engine/Graphical2D) | the window, the keyboard, the mouse, 2D sprites and texts |
| `engine-graphical3D` | [`Engine/Graphical3D`](../Engine/Graphical3D) | 3D models and the camera |
| `engine-audio` | [`Engine/Audio`](../Engine/Audio) | sounds and the point that hears them |
| `game` | [`Engine/Game`](../Engine/Game) | the engine's executable: the `main`, the modules and everything linked to Lua |

In `game`, everything is in the `RType::Game` namespace, one class per role:

| File | Role |
|---|---|
| `Runner` | the engine's `main`: modules, scripts, loop |
| `ConfigBuilder` | reading `config.conf`, which updates `GameConfig` |
| `Modules` | the modules a game can ask for |
| `Scripts` | loading the scripts and converting their errors |
| `GameManagerBinding` | exposes the `GameManager`, the entities and `gm:stop`, then calls the other bindings |
| `ComponentBinding` | the registry of the components usable from Lua |
| `SystemBinding` | `gm:createSystem`, `gm:addSystem`, `gm:destroySystem`, `gm:hasSystem` |
| `ViewBinding` | `gm:view` |
| `InputBinding` | `gm:keyboard`, `gm:mouse`, the `Key` and `MouseButton` tables |
| `TimeBinding` | `gm:elapsed` |
| `WindowBinding` | `gm:shouldClose`, `gm:fps`, `gm:setFps`, `gm:fullScreen`, `gm:setFullScreen`, `gm:hideCursor`, `gm:setHideCursor` |
| `AudioBinding` | `gm:setListenerPosition` |
| **`EngineBindings.cpp`** | **the lists of the engine's components and systems usable from Lua: the file to edit to add one** |

---

## 2. Adding a component

### Writing the component

A component is a data struct, placed in the `include` folder of the library it belongs to. Give every field a default value: it is what a script gets when it does not give the field.

```cpp
// Engine/System/include/BuiltIn/Health.hpp
namespace RType {
    struct Health {
        int hp = 100;
        int max = 100;
    };
}
```

If it has a `.cpp` (for its systems, for instance), add it to its library's `SRCS_...` list in [`Engine/CMakeLists.txt`](../Engine/CMakeLists.txt). The rules of a good component (movable, no reference to its own entity…) are in [the ECS guide](ECS_GUIDE.en.md#3-components).

### Making it usable from Lua

Add one `bindComponent` line to `ComponentBinding::bindEngineComponents`, in [`EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp): the C++ type, the name scripts use, then each field as `"name", &Type::member`.

```cpp
bindComponent<Health>(lua, "Health", "hp", &Health::hp, "max", &Health::max);
```

This single line gives the component, on the Lua side:

- `gm:addComponent(e, 'Health', {hp = 50})`, with the default values for the fields left out;
- the field checks, with their messages: `no field 'hpp'`, `field 'hp' expects a number, got a string`;
- `gm:getComponent`, `gm:hasComponent`, `gm:removeComponent`, `gm:countComponents`;
- its use in the list of a `gm:createSystem` and of a `gm:view`.

Changing a field only means changing this line.

### Fields that are not plain members

| Case | How to write it | Example |
|---|---|---|
| a field of a field | `nested` | `"rows", nested(&SpriteSheet2D::recNb, &Size2D::height)` |
| a `std::chrono::milliseconds` | `milliseconds`, which exposes it as a number of milliseconds | `"frameDuration", milliseconds(&SpriteSheet2D::frameDuration)` |
| a method | a lambda that takes the component first | `"elapsed", [](const Clock &c) { return c.getElapsed().count(); }` |
| a method of the component itself | a method pointer | `"setVolume", &Sound::setVolume` |
| a member of a parent class | `sol::property`, with one lambda to read and one to write | `"loop", sol::property([](const Sound &s) { return s.loop; }, [](Sound &s, bool v) { s.loop = v; })` |
| an enum | declare its values with `lua.new_enum`, at the top of `bindEngineComponents`; the field is then written normally | `lua.new_enum<SoundInfo::State>("SoundState", {{"PLAY", SoundInfo::State::PLAY}, ...})` |

A method cannot be given to `addComponent`: `{elapsed = 5}` is refused with `field 'elapsed' expects a function, got a number`.

### A component built from a file

A component **without a default constructor**, such as `Texture2D`, is built with the script's `path` field, then given the table's other fields. Its line does not change:

```cpp
bindComponent<Texture2D>(lua, "Texture2D");
```

If its constructor needs **more than the path**, specialize `ComponentBinding::construct` in `EngineBindings.cpp`, before `bindEngineComponents`. It gets the `GameManager`, to fetch what is missing. This is what `Sound` does, as it needs the point that hears the sounds, created by the `Audio` module:

```cpp
template <>
Sound ComponentBinding::construct<Sound>(GameManager &gm, const std::string &path)
{
    return Sound(GameManagerBinding::findModuleComponent<Listener>(gm, "Sound", "Audio"), path);
}
```

### Documenting it

Add a row to the table of [the components of the Lua guide](LUA_GUIDE.en.md#the-components), in French and in English: its name, its fields with their default value, and its role.

---

## 3. Adding a system

### Writing the system

An engine system is a static function of the main component's class, named `system` followed by what it does. Its parameters are the components it works on, `const` for the ones it only reads, possibly with the `GameManager` first:

```cpp
struct Health {
    int hp = 100;
    int max = 100;

    static void systemRegenerate(Health &health, const Clock &clock);
};
```

The engine calls it every frame, once for each entity that has all these components.

### Making it usable from Lua

Add one line to `SystemBinding::engineSystems`, in [`EngineBindings.cpp`](../Engine/Game/src/EngineBindings.cpp): the name scripts use, which is the function's without its `system` prefix, then `makeSystem` with the function.

```cpp
{"Regenerate", makeSystem(Health::systemRegenerate)},
```

`makeSystem` reads the components from the function's own parameters: there is nothing else to write. A script then adds it with `gm:addSystem('Regenerate')`.

A system that belongs to a module (see [section 4](#4-adding-a-module)) **is not** in this list: the module registers it once, and a script must not be able to run it a second time.

### Documenting it

Add a row to the table of [the engine's systems of the Lua guide](LUA_GUIDE.en.md#the-engines-systems), in French and in English: its name, the components it acts on, and its role.

---

## 4. Adding a module

A module is a feature the game turns on by its name, in the `modules` key of its `config.conf`. It creates, once and before the scripts, an entity with the component it owns, and the system that updates it. This component belongs to the engine: a script reads it, but can neither create another one nor add the system a second time.

### Writing the module

Add an entry to `Modules::allowed`, in [`Modules.cpp`](../Engine/Game/src/Modules.cpp): its name, the modules it depends on, and what it creates.

```cpp
{"Audio",
    {{},
        [](GameManager &gm, const GameInfo &) {
            gm.addComponent<Listener>(gm.createEntity());
            gm.createSystem<Sound>(Sound::systemSound);
        }}},
```

- **The dependencies** are checked before anything runs: `{"Window"}` for a module that needs the window, like `Keyboard`.
- **Its component has no `bindComponent`**, and **its systems are not in `engineSystems`**. A script that tries to create them gets `Unknown component` or `Unknown system`.
- The lambda gets `GameConfig`, filled from `config.conf`, for a module that has settings, like the window's size for `Window`.

### Giving scripts access to its component

A script reaches a module's component only through a `GameManager` function, such as `gm:keyboard()` or `gm:elapsed()`. They are grouped in a binding class, on the model of [`InputBinding`](../Engine/Game/include/InputBinding.hpp), [`TimeBinding`](../Engine/Game/include/TimeBinding.hpp) and [`WindowBinding`](../Engine/Game/include/WindowBinding.hpp):

1. **A header** in `Engine/Game/include`, with a class of the `RType::Game` namespace whose only public function is the static `registerFunctions(sol::state &lua)`.
2. **A `.cpp`** in `Engine/Game/src`, that implements it:

   ```cpp
   void InputBinding::registerFunctions(sol::state &lua)
   {
       lua.new_usertype<KeyboardManager>("KeyboardManager", sol::no_constructor,
           "isDown", contains(&KeyboardManager::keyDown));
       lua["GameManager"]["keyboard"] = [](GameManager &gm) -> KeyboardManager & {
           return GameManagerBinding::findModuleComponent<KeyboardManager>(
               gm, "gm:keyboard", "Keyboard");
       };
   }
   ```

   - **`sol::no_constructor`** prevents a script from writing `KeyboardManager.new()`.
   - **`GameManagerBinding::findModuleComponent`** finds the component created by the module. Without the module, it throws `ModuleNotLoaded`, which the script gets as `gm:keyboard needs the Keyboard module`.
   - **Returning a reference** (`-> KeyboardManager &`) gives the script the real component, updated every frame.
   - If the script only needs a value, return it directly, without exposing the component: `gm:elapsed()` returns the `MasterClock`'s time without exposing the `MasterClock`.
3. **The call** to `registerFunctions` in `GameManagerBinding::registerFunctions`.
4. **The `.cpp`** in the `SRCS_GAME` list of [`Engine/CMakeLists.txt`](../Engine/CMakeLists.txt).

### Documenting it

- A row in the table of [the modules of the guide to running a game](ENGINE_GAME.en.md#3-the-modules), in French and in English: its name, what it creates, how scripts reach it, and its dependencies.
- A subsection in [the Lua guide](LUA_GUIDE.en.md), with an example, and its functions in the [cheat sheet](LUA_GUIDE.en.md#11-api-cheat-sheet).

---

## 5. Adding a Lua function to the `GameManager`

Any function callable as `gm:name(...)` is added in the `registerFunctions` of a binding class, with a lambda that takes the `GameManager` first:

```cpp
lua["GameManager"]["countComponents"] =
    [](GameManager &gm, const std::string &name) {
        return getEntry(name).count(gm);
    };
```

- **Errors** are reported by throwing an exception that inherits from `EngineException`, declared in [`Exceptions.hpp`](../Engine/System/include/Exceptions.hpp). The script gets it as a Lua error, with its message, and the game stops with this message if the script does not catch it with `pcall`.
- **Arguments of the wrong type** are refused by sol, with no code to write: `stack index 2, expected number, received string`.
- **A function that creates a system** goes in `SystemBinding` and calls `checkNotUpdating` first, to refuse being called from a system.

---

## 6. Summary

| To add | Code | Lua | Doc |
|---|---|---|---|
| a component | the struct, and its `.cpp` in `SRCS_...` | one `bindComponent` line in `EngineBindings.cpp` | the components table of the Lua guide |
| a system | a static function `systemName` | one `makeSystem` line in `engineSystems`, unless it belongs to a module | the engine's systems table of the Lua guide |
| a module | an entry in `Modules::allowed` | a binding class with `findModuleComponent`, called by `GameManagerBinding` | the modules table of the guide to running a game, and a subsection of the Lua guide |
| a `gm:` function | | a lambda in the `registerFunctions` of a binding class | the Lua guide's cheat sheet |
| a `config.conf` setting | a field of `GameInfo` | an entry in `ConfigBuilder::setters`, which reads the value and updates the field | the `config.conf` file of the guide to running a game |
