# ECS Design Notes

> 🌐 **Language:** [Français](ECS_DESIGN.fr.md) · English

This document describes the architecture of the engine's ECS (`Engine/System`) and, above all, **the reasons** behind its design choices.

- **Audience:** people maintaining or extending the engine.
- **To use the ECS day to day**, see [the ECS user guide](ECS_GUIDE.en.md) instead.

Only signatures are shown, never function bodies: this document describes an architecture, not an implementation. Signatures are simplified (Doxygen comments, `[[nodiscard]]` and deleted copy constructors are omitted); the code in `Engine/System/include/` remains the reference.

Changes that are considered but not implemented are listed at the end, in [What is left to do](#what-is-left-to-do).

---

## Table of contents

- [Guiding principles](#guiding-principles)
- [Overview](#overview)
- [Current architecture](#current-architecture)
  - [Entity and EntityManager](#entity-and-entitymanager)
  - [Components](#components)
  - [IComponentArray and ComponentArray](#icomponentarray-and-componentarray)
  - [ComponentManager](#componentmanager)
  - [ISystem, System and SystemManager](#isystem-system-and-systemmanager)
  - [GameManager (facade)](#gamemanager-facade)
  - [The command queue](#the-command-queue)
- [What is left to do](#what-is-left-to-do)

---

## Guiding principles

1. **Components are plain data, with no common base class.** There is no `Component` class. Inheritance and polymorphism would destroy the memory locality we are after (contiguous storage per type) and would force pointer-based storage (`unique_ptr<Component>`) instead of dense containers.

2. **One `ComponentArray<Component>` per component type**, organized as a *sparse set*: `dense` holds the actual data, packed and fast to iterate; `sparse` gives direct O(1) access from an `Entity`. The two roles are deliberately separate: `dense` only contains components that exist, while `sparse` may contain holes (it is sized on the range of entity indices).

3. **A virtual function can never depend on the `Component` type.** This rule drives the whole split between interfaces (`IComponentArray`, `ISystem`) and template classes (`ComponentArray<Component>`, `System<Components...>`): a virtual function must have a fixed signature, known at compile time to build the vtable, so it cannot be a template. Everything that handles the concrete type lives only on the template class, never on the interface.

4. **Entities are generational handles, not objects.** An `Entity` is an *(index, generation)* pair. The index is recycled; the generation guarantees that an old handle will never refer to the new entity.

5. **Every long-lived reference goes through a stable container.** Components are stored in a `plf::colony` and systems in a `std::list`: an element never moves as long as it is not itself removed.

6. **No structural change during an iteration.** During `update()`, operations that change the structure (destruction, addition, removal) are deferred to a command queue and applied at the end of the frame.

7. **`GameManager` is the only public facade.** `EntityManager`, `ComponentManager` and `SystemManager` are internal details; game code goes exclusively through `GameManager`.

---

## Overview

```text
GameManager                      public facade + command queue
├── ComponentManager             component type -> ComponentArray
│   └── ComponentArray<T>        sparse set (dense: plf::colony<ComponentPack>)
├── EntityManager                index + generation, index recycling
└── SystemManager                std::list of systems, execution order
    └── System<Ts...>            calls a function on every entity that has all the Ts
```

Every class lives in the `RType` namespace.

---

## Current architecture

### Entity and EntityManager

```cpp
class Entity {
public:
    explicit Entity(std::size_t index, std::size_t generation = 0);

    bool operator==(const Entity &entity) const;
    std::size_t index() const;
    std::size_t generation() const;

private:
    std::size_t _index;
    std::size_t _generation;
};

struct EntityInfo {
    bool isActive = true;
    std::size_t generation = 0;
};

using EntityList = std::vector<EntityInfo>;
using RecycleQueue = std::queue<Entity>;

class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

private:
    EntityList _entities;      // one slot per index ever handed out
    RecycleQueue _toRecycle;   // freed indices, waiting to be reused
};
```

**Why a generation on top of the index.** Without a generation, a handle kept after its entity was destroyed would silently refer to the next entity that gets the same index: a missile could, for example, start chasing an unrelated target. With the *(index, generation)* pair, `hasEntity` compares the handle's generation with the slot's: a stale handle is detected instead of being reinterpreted.

**Why a FIFO queue for recycling.** The index freed the longest ago is reused first. A given index is therefore reused as late as possible, which limits how often its generation is incremented.

**Why `Entity` has no default constructor, no `operator<` and no `std::hash`.** The lack of a default constructor prevents creating an "empty" handle that would implicitly refer to index 0. Ordering and hashing have not been needed so far; they can be added if a need arises.

> [!NOTE]
> The `Entity(index, generation)` constructor is public for technical reasons (it is used by `EntityManager`), but game code must never call it: an `Entity` is only obtained through `createEntity()`.

### Components

```cpp
struct Position {
    float x;
    float y;
};
```

No base class, no custom constructor or destructor is needed: an ordinary struct is enough. The only requirement is being **movable**; non-copyable types are accepted.

### IComponentArray and ComponentArray

```cpp
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual std::size_t nbComponent() const = 0;
    virtual void removeEntity(Entity entity) = 0;
    virtual bool hasComponent(Entity entity) const = 0;
};

template<typename Component>
class ComponentArray : public IComponentArray {
public:
    class ComponentPack {
    public:
        explicit ComponentPack(Entity entity, Component &&c);
        Entity entity() const;

        Component component;

    private:
        Entity _entity;
    };

    using Dense = plf::colony<ComponentPack>;

    template<typename... Args>
    Component &addEntity(Entity entity, Args &&...args);
    void removeEntity(Entity entity) override;
    Component &getComponent(Entity entity);
    bool hasComponent(Entity entity) const override;
    std::size_t nbComponent() const override;
    Dense &getDense();

private:
    Dense _dense;
    std::vector<typename Dense::iterator> _sparse;   // indexed by entity.index()
};
```

**Why `plf::colony` rather than `std::vector` for `dense`.** With a `std::vector`, a removal requires a *swap-and-pop* that moves the last element: any external reference to that component becomes invalid, and `sparse` must be resynchronized. `plf::colony` (the reference implementation of `std::hive`, standardized in C++26) guarantees that an element that is not removed never moves, regardless of insertions and removals elsewhere in the container. This is what allows `sparse` to store iterators directly, and game code to keep a reference to a component (see the user guide, section 3).

**How `sparse` represents a missing component.** A `sparse` slot holds either a valid iterator into `dense`, or a default-constructed iterator meaning "no component". `sparse` grows on demand, to about 1.5 times the requested index, to amortize resizes.

**Why `dense` stores the entity with the component (`ComponentPack`).** A system over several components (`System<Position, Velocity>`) iterates over the `dense` of one of its types, then has to find the other components **of the same entity**. But `sparse` only works in the entity → component direction: without a link from the element to its entity, the system cannot know who owns the component it is iterating over. A separate entity array parallel to `dense` (EnTT's choice) is not possible with `plf::colony`, which offers no stable index shared between two containers; the entity is therefore stored in the element itself. The overhead is at least 16 bytes per component, acceptable for the number of entities in an R-Type.

**Why the whole `Entity`, and not just its index.** The index would be enough to query the other arrays, but a handle rebuilt from the index alone would have a wrong generation: it would be considered stale as soon as it left the array (towards a system or towards `GameManager`). Storing the complete `Entity` guarantees that `pack.entity()` is always a real handle.

**Why the entity is read-only.** `getDense()` is public. If the entity of a `ComponentPack` could be modified, `dense` and `sparse` would get out of sync. It is therefore private and exposed through an accessor; only `component` can be modified.

**Why `ComponentArray` does not check the generation.** `sparse` is indexed by `entity.index()` only. The generation check is done upstream, once, by `GameManager` (`hasEntity`); and since destroying an entity removes all of its components, a recycled slot never holds the previous entity's components. **Invariant:** no code may call `ComponentArray` with an unchecked handle.

**Why `hasComponent`, `removeEntity` and `nbComponent` are virtual, but not `addEntity` or `getComponent`.** The first three have a signature independent of `Component`: they can legitimately live on the interface. `addEntity` and `getComponent` handle a `Component &`, a different type for each `ComponentArray`: they structurally cannot be virtual (principle 3). `ComponentManager` calls them directly on the concrete type.

### ComponentManager

```cpp
using ComponentList =
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>>;

class ComponentManager {
public:
    template<typename Component, typename... Args>
    Component &addComponent(Entity entity, Args &&...args);

    template<typename Component>
    void removeComponent(Entity entity);

    void removeComponents(Entity entity);   // clears ALL ComponentArrays for this entity

    template<typename Component>
    Component &getComponent(Entity entity);

    template<typename Component>
    ComponentArray<Component> &getComponentArray();

    template<typename Component>
    bool hasComponent(Entity entity) const;

    template<typename Component>
    bool isComponent() const;   // does a ComponentArray<Component> exist?

private:
    ComponentList _components;
};
```

**Why a `std::unordered_map<std::type_index, …>`.** `getComponentArray<Component>()` must find the array matching a given type. With a plain `std::vector`, it would have to walk the whole list trying a `dynamic_cast` on each element: slow and fragile. With a table keyed by `std::type_index(typeid(Component))`, the lookup is direct.

**How the concrete type is recovered.** `getComponentArray<Component>()` looks up the entry under the key `type_index(typeid(Component))`, then performs a `static_cast<ComponentArray<Component> &>` on the stored `IComponentArray`. This cast is safe only because `ComponentManager` upholds the following invariant: an entry under that key is never created as anything other than a real `ComponentArray<Component>` (in `addComponent<Component>`, which creates the array if it does not exist yet).

**Why an array is never removed.** Once created, a `ComponentArray<Component>` stays in place even when empty. References obtained through `getArray<T>()` therefore stay valid, and `isComponent<T>()` never goes back to `false`.

**Why `removeComponents(Entity)` exists alongside `removeComponent<Component>`.** When an entity is destroyed, its components must be removed from *every* array, without knowing their types at that point in the code. This is the main use case for polymorphism: we iterate over `_components` and call `hasComponent` then `removeEntity` through the interface.

### ISystem, System and SystemManager

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(ComponentManager &cm) = 0;
};

template<typename Component, typename... Components>
constexpr std::size_t duplicateCheck = (std::is_same_v<Component, Components> + ...);

template<typename... Components>
class System : public ISystem {
public:
    static_assert(((duplicateCheck<Components, Components...> == 1) && ...));

    explicit System(std::function<void(Components &...)> &func);
    void update(ComponentManager &cm) override;   // calls _func on every entity that has all the Components

private:
    std::function<void(Components &...)> _func;

    void iterateSmallest(ComponentArray<Components> &...arrays);
    template<typename Array>
    void iterate(Array &smallest, ComponentArray<Components> &...arrays);
};

using SystemId = std::size_t;

struct SystemEntry {
    SystemId id;
    std::unique_ptr<ISystem> system;
};

using SystemList = std::list<SystemEntry>;

class SystemManager {
public:
    void process(ComponentManager &cm);

    template<typename... Components>
    SystemId createSystem(std::function<void(Components &...)> &func);

    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;

private:
    SystemList _systems;
    SystemId _nextId = 0;
};
```

**Why `update` takes a `ComponentManager &`.** A system needs access to typed data (the `ComponentArray` of each of its types), not to a raw list of identifiers. `ComponentManager` is the only relevant access point. If one of the types has never existed (`isComponent<T>()` is false for one of them), the system does nothing.

**How `System<Components...>` finds the entities (the join).**

1. `update` fetches the `ComponentArray` of every type once, then passes them to `iterateSmallest` as a parameter pack.
2. `iterateSmallest` computes the smallest size (`std::min` over the `nbComponent()`), then calls `iterate` on the **first array of that size**, and on it only (fold over `||`, which stops at the first `true`).
3. `iterate` iterates over the `dense` of that array; each `ComponentPack` gives the element's entity. If the entity has all the other types, the function is called with one component of each type, obtained in O(1) directly from the arrays it received (without going through `ComponentManager` again). Otherwise, the entity is skipped.

**Why iterate over the smallest array.** Each lookup is O(1), but there is one per element of the iterated array: the cost of a frame therefore depends on the size of that array, not on the size of the others. For `System<Position, Player>` with 1000 positions and a single player, iterating over `Position` would cost 1000 lookups, 999 of them useless; iterating over `Player` costs only one. The choice is made again on every `update()`, since sizes change during the game.

**Why forbid a duplicated type, and at compile time.** With `System<Position, Position>`, the function would receive two references to **the same** component: a source of silent bugs (two writes overwriting each other), with no legitimate use case. A `static_assert` turns this bug into a compile-time error, with a readable message. `duplicateCheck<T, Components...>` counts the occurrences of `T` in the pack (a fold over `+` of `std::is_same_v`); the outer fold over `&&` requires every type to appear exactly once. Comparing each type with the first one only would not be enough: `<Position, Velocity, Velocity>` would get through.

**Why `iterate` is a template.** The array to iterate over is only known at runtime, but its type (`ComponentArray<Position>` or `ComponentArray<Player>`) must be known at compile time: no variable can refer to "one or the other". The loop is therefore written once, as a template over the array type, and the fold instantiates it for each possible type; only the instance matching the smallest array is called.

**Why `createSystem` returns a `SystemId` rather than a pointer or an iterator.**

- Returning a `std::unique_ptr<ISystem>` would transfer ownership to the caller, but `SystemManager` must remain the sole owner in order to run the system in `process()`.
- A raw pointer or an iterator would become invalid as soon as the system is destroyed, with no way to detect it.
- A numeric id, on the other hand, can always be checked (`hasSystem`). Since ids are handed out by an increasing counter (`_nextId`) and **never reused**, an old id can never refer to another system.

**Why a `std::list`.**

- **Execution order is creation order**: systems are appended to the list and iterated in order.
- **Iterators stay valid when an element is added**: a `createSystem` call during `process()` does not corrupt the ongoing iteration (the new system simply runs in that same frame). With a `std::vector`, a reallocation would invalidate the iteration.
- Lookup by id is linear, which is acceptable: the number of systems is small and `destroySystem`/`hasSystem` are not called every frame.

**Why destroying a system is deferred during `update()`.** Removing a node from the list while iterating over it would invalidate the current iterator. `GameManager` therefore puts `destroySystem` in the command queue (see below).

### GameManager (facade)

```cpp
using Command = std::function<void()>;
using CommandQueue = std::vector<Command>;

class GameManager {
public:
    void update();

    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

    template<typename Component, typename... Args>
    void addComponent(Entity entity, Args &&...args);
    template<typename Component>
    void removeComponent(Entity entity);
    void removeComponents(Entity entity);
    template<typename Component>
    Component &getComponent(Entity entity);
    template<typename Component>
    bool hasComponent(Entity entity) const;
    template<typename Component>
    ComponentArray<Component> &getArray();

    template<typename... Components>
    SystemId createSystem(
        std::type_identity_t<std::function<void(Components &...)>> func);
    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;

private:
    ComponentManager _cm;
    EntityManager _em;
    SystemManager _sm;
    CommandQueue _cq;
    bool _updating = false;

    void flush();
    void tryFlush();
};
```

**Why a single facade.** It is the only class game code needs to know about. It also centralizes cross-cutting checks: every component access first checks `hasEntity` (and therefore the generation), which lets the internal classes skip it.

**Why copying is disabled.** Pending commands and system functions often capture `this` or a reference to the `GameManager`. A copy would share those captures with the original, leading to accesses on the wrong object.

**Why `std::type_identity_t` in `createSystem`.** The types are given explicitly (`createSystem<Position, Velocity>(lambda)`), but for a parameter pack, the compiler still tries to deduce more of them from the argument. Since a lambda is not a `std::function`, that deduction fails and the call does not compile. `std::type_identity_t` makes the parameter non-deduced: only the explicit types are used, and the lambda is converted to a `std::function`.

### The command queue

**The problem.** During `update()`, each system iterates over the `dense` of a `ComponentArray`. If a system destroyed an entity or added a component during that iteration, it would modify the container being read, or the system list itself.

**The solution.** Structural operations (`destroyEntity`, `addComponent`, `removeComponent`, `removeComponents`, `destroySystem`) do not run directly: they push a `Command` (a lambda) into `_cq`, then call `tryFlush()`.

- **Outside `update()`** (`_updating` is false), `tryFlush()` runs the queue immediately: the operation looks synchronous.
- **During `update()`**, `tryFlush()` does nothing; the queue is run by `flush()` at the end of `update()`, in request order.

Non-structural operations (`createEntity`, reads) remain immediate. `createEntity` can safely be immediate because it does not touch any container iterated by systems; and since destructions are deferred, an index freed during the frame is never handed out again within that same frame.

**Why checks are done inside the command, not at call time.** State can change between the request and its execution (for example, two systems destroy the same entity). Each command therefore re-checks `hasEntity`/`hasSystem` when it runs: a double destruction has no effect, and an addition on an entity destroyed in the meantime throws `NoSuchEntity`.

**Why `addComponent` constructs the component immediately.** The component is constructed at call time, then moved into its array when the command runs. An exception thrown by the constructor therefore comes straight out of the `addComponent` call, where it makes sense. The component is held in a `std::shared_ptr` captured by the lambda, because `std::function` requires copyable callables: directly capturing a non-copyable component (holding a `std::unique_ptr`, for example) would not compile.

**Exception guarantees.**

- `flush()` runs **every** command, even if some fail; it keeps the first exception encountered and rethrows it after clearing the queue.
- If a system throws, `update()` still runs the queue (ignoring command errors) and then rethrows the system's exception. The frame is thus always applied, and the ECS stays in a consistent state for the next frame.
- `update()` is not reentrant: a call from within a system (`_updating` is true) has no effect.

> [!WARNING]
> A command must never call back into the `GameManager` while it runs, and this includes component constructors and destructors: pushing a command into `_cq` while `flush()` is iterating over that same vector would cause a reallocation mid-iteration, or infinite recursion of `flush()`. This is the rule documented in the user guide (section 7, pitfall 1).

---

## What is left to do

These changes are **not implemented**. Only their principle is described here.

### A pointer instead of an iterator in `sparse`

`sparse` currently stores one `plf::colony::iterator` per entity index, that is 24 bytes per slot, for every component type, including for entities that do not have that component. A plain `ComponentPack *` (8 bytes, `nullptr` for "no component") would be enough: `plf::colony` guarantees that an element does not move as long as it is not removed. Only `removeEntity` needs an iterator, for `erase`: it can get it back with `_dense.get_iterator(pointer)`, an operation slower than a direct access, but rare and always deferred.

### Enabling and disabling an entity

Allow pausing an entity (freezing an enemy, a cutscene, a death animation…), for every system or for a single one, without removing its components. The prerequisite is met, since each `ComponentPack` knows its entity: a system can therefore check, for each entity it iterates over, whether a disabling "tag" component is present. Two points remain to be designed:

- **how to refer to a specific system**: `System<Component, Tag>` is no longer possible, since `System` is variadic and would read it as a system over two components. A distinct class will be needed, or the system will have to be referred to by its `SystemId`;
- **filtering on the absence of a component**: a multi-component system already filters on the *presence* of a tag, but not yet on its *absence*.

In the meantime, adding and removing a "tag" component covers most cases (see the "Pausing an entity" recipe in the user guide, section 8).
