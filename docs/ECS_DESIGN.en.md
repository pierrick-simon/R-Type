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
  - [View](#view)
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
├── View<Ts...>                  iterates over the entities that have all the Ts (join)
├── EntityManager                index + generation, index recycling
└── SystemManager                std::list of systems, execution order
    └── System<Ts...>            loop over a View<Ts...>; no type: once per frame
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

### View

```cpp
template<typename Component, typename... Components>
constexpr std::size_t duplicateCheck = (std::is_same_v<Component, Components> + ...);

template<typename... Components>
class View {
public:
    static_assert(((duplicateCheck<Components, Components...> == 1) && ...));

    explicit View(ComponentManager &cm);
    void each(std::function<void(Entity, Components &...)> f);
    std::vector<std::tuple<Entity, Components &...>> getVector();

private:
    std::tuple<ComponentArray<Components> *...> _view;   // nullptr if the type has never existed

    void iterateSmallest(std::function<void(Entity, Components &...)> &f,
        ComponentArray<Components> &...arrays);
    template<typename Array>
    void iterate(std::function<void(Entity, Components &...)> &f, Array &smallest,
        ComponentArray<Components> &...arrays);
};
```

**Why a view.** Iterating over the entities that have *several* component types (the *join*) is useful everywhere: in a system, in a frame system (collisions), or outside any frame (network, rendering). The view makes this join available anywhere, and provides the entity on top of the components. `System<Components...>` is now just a loop over a view.

**How the view finds the entities (the join).**

1. The constructor fetches, once, a pointer to the `ComponentArray` of each type.
2. `each` opens the tuple with `std::apply` to get the arrays as a pack, then calls `iterateSmallest`.
3. `iterateSmallest` computes the smallest size (`std::min` over the `nbComponent()`), then calls `iterate` on the **first array of that size**, and on it only (fold over `||`, which stops at the first `true`).
4. `iterate` iterates over the `dense` of that array; each `ComponentPack` gives the element's entity. If the entity has all the other types, the function is called with the entity and one component of each type, obtained in O(1) directly from the arrays (without going through `ComponentManager` again). Otherwise, the entity is skipped.

**Why iterate over the smallest array.** Each lookup is O(1), but there is one per element of the iterated array: the cost therefore depends on the size of that array, not on the size of the others. For `View<Position, Player>` with 1000 positions and a single player, iterating over `Position` would cost 1000 lookups, 999 of them useless; iterating over `Player` costs only one. The choice is made again on every `each`, since sizes change during the game.

**Why `iterate` is a template.** The array to iterate over is only known at runtime, but its type (`ComponentArray<Position>` or `ComponentArray<Player>`) must be known at compile time: no variable can refer to "one or the other". The loop is therefore written once, as a template over the array type, and the fold instantiates it for each possible type; only the instance matching the smallest array is called.

**Why pointers, stored in a `std::tuple`.**

- **Pointers rather than references:** if a type has never been added, `getComponentArray<T>()` would throw. The view then stores `nullptr`, and `each` does nothing: a view on a missing type is empty, without exception. A reference cannot mean "nothing".
- **A tuple:** a pack cannot be a class member (`ComponentArray<Components> *...arrays;` is not valid C++). The tuple stores one pointer per type, and `std::apply` turns it back into a pack when iterating.

**Why forbid a duplicated type, and at compile time.** With `View<Position, Position>`, the function would receive two references to **the same** component: a source of silent bugs (two writes overwriting each other), with no legitimate use case. A `static_assert` turns this bug into a compile-time error, with a readable message. `duplicateCheck<T, Components...>` counts the occurrences of `T` in the pack (a fold over `+` of `std::is_same_v`); the outer fold over `&&` requires every type to appear exactly once. Comparing each type with the first one only would not be enough: `<Position, Velocity, Velocity>` would get through. `System` carries the same `static_assert`, so that the error points at `createSystem` rather than at an internal view.

**Why a view is not a cache.** It only stores pointers to the arrays, never a list of entities: it therefore always sees the current state, and creating one only costs a few lookups in `ComponentManager`. It never needs to be invalidated or updated, but it is meant to be used right away, then thrown away.

**Why `getVector` exists, and why it reuses `each`.** `each` allows neither `break`, nor `return` to the caller, nor iterating twice over the same list (the nested loops of collisions). A range-for allows all of them, but it requires `begin()`/`end()`: writing a real iterator is hard here, because the array iterated over is chosen at runtime among `Dense` storages of different types (see [What is left to do](#what-is-left-to-do)). `getVector` sidesteps the problem: it calls `each` with a lambda that appends each entity and its components to the vector, so the join is written only once. The range-for then iterates over a plain `std::vector`.

**Why references in the tuple.** `std::tuple<Entity, Components...>` would copy every component: a write in the loop would change the copy, not the component, and a non-copyable component would not compile. A tuple of references is legal in a `std::vector` (it can be constructed and moved, which is all `emplace_back` needs). Since the vector's elements are lvalues, `for (auto &[e, a, b] : ...)` compiles, unlike with an iterator that would return the tuple by value.

**What `getVector` costs, and why it is safe.** One allocation per call, and a snapshot of the state at the time of the call. The references stay valid as long as their component is not removed, since `plf::colony` never moves an element: destroying an entity during the iteration (even outside `update()`, where it is immediate) only invalidates that entity's references, never the vector itself. This is why the guide recommends it to destroy entities outside `update()`, instead of "collect then destroy".

**Why `each` takes a `std::function`.** It is the simplest form, and it accepts `const` parameters. The cost is one indirect call per entity; a template parameter (`template<typename F> void each(F &&f)`) would avoid it (see [What is left to do](#what-is-left-to-do)).

### ISystem, System and SystemManager

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(ComponentManager &cm) = 0;
};

template<typename... Components>
class System : public ISystem {
public:
    static_assert(((duplicateCheck<Components, Components...> == 1) && ...));

    explicit System(std::function<void(Components &...)> &func);
    void update(ComponentManager &cm) override;   // a View<Components...>, or _func() once if no type

private:
    std::function<void(Components &...)> _func;
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

**How `System<Components...>` finds the entities.** `update` creates a `View<Components...>` and calls `each` with a lambda that ignores the entity and calls `_func` with the components. The whole join (smallest array, missing types, duplicates) is therefore the view's: see [View](#view).

**Why a system without types is a frame system.** Some processing does not concern *one* entity: collisions between two groups, enemy waves spawning, the score. Rather than a second API, an empty pack takes on that meaning: with `Components` empty, `std::function<void(Components &...)>` naturally becomes `std::function<void()>`, and `update` calls `_func()` only once. The system keeps everything else: its `SystemId`, its rank in execution order, and the protection of the command queue. Thanks to `std::type_identity_t`, the pack is deduced as empty when no type is given: `createSystem(func)` and `createSystem<>(func)` are equivalent. A frame system can also receive the `GameManager` (see [GameManager (facade)](#gamemanager-facade)): it is the recommended form for collisions, or for any system that needs the entity it processes, by iterating over a view.

**Why `if constexpr` in `update`.** With a plain `if`, both branches would be compiled for **every** `System`: `_func()` would not compile for a `System<Health>` (its function expects a `Health &`), and `View<>` would not compile for a `System<>` (`std::min` over an empty list). `if constexpr` discards from compilation the branch that does not match the pack; the `else` is required for the second branch to be discarded too.

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
    View<Components...> view();

    template<typename... Components>
    SystemId createSystem(
        std::type_identity_t<std::function<void(Components &...)>> func);
    template<typename... Components>
    SystemId createSystem(const std::type_identity_t<
        std::function<void(GameManager &, Components &...)>> &func);
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

**Why copying is disabled.** Pending commands and the adapters of systems that receive the `GameManager` capture `this`. A copy would share those captures with the original, leading to accesses on the wrong object. Since no move constructor is declared, a `GameManager` cannot be moved either: its address never changes, and the captured `this` stays valid as long as it lives.

**Why a system can receive the `GameManager`.** A system that acts on the world (creating or destroying entities, reading another entity's components, iterating over a view) needs the `GameManager`. Capturing it in a lambda (`[&gm]`) works, but prevents writing the system as a plain function, in a separate file. `createSystem` therefore also accepts a function whose first parameter is a `GameManager &`.

- **Two overloads, without ambiguity.** The second overload takes a `std::function<void(GameManager &, Components &...)>`. The `std::function` constructor only accepts functions callable with its arguments: for a given lambda or function, only one of the two overloads is viable, and the compiler picks it on its own. A generic lambda (`[](auto &...)`), which would accept both forms, makes the call ambiguous: it is a compile-time error, which avoids an arbitrary choice.
- **An adapter, built once.** The `GameManager` is known as soon as the system is created: it is `this`. The second overload therefore wraps the function in a lambda of the usual `(Components &...)` form, which calls `func(*this, c...)`. `System` and `SystemManager` only see the usual form and did not change; with an empty pack, the adapter becomes a lambda without parameters, and the frame system works with no special case.
- **The function is captured by value.** `func` is a parameter of `createSystem`, destroyed as soon as it returns; the adapter, on the other hand, lives as long as the system. It must therefore keep its own copy of the function: a capture by reference would leave a dangling reference, called on the first frame.

**Why `std::type_identity_t` in `createSystem`.** The types are given explicitly (`createSystem<Position, Velocity>(lambda)`), but for a parameter pack, the compiler still tries to deduce more of them from the argument. Since a lambda is not a `std::function`, that deduction fails and the call does not compile. `std::type_identity_t` makes the parameter non-deduced, in both overloads: only the explicit types are used, and the lambda is converted to a `std::function`.

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

### Receiving the entity in a system's function

Allow `createSystem<Lifetime>([](Entity e, Lifetime &l) {...})`, alongside the current forms. In the meantime, a frame system iterating over a view already provides the entity: it is the practice recommended by the user guide, and it avoids storing the entity in its own components. The difficulty is accepting **several** function forms, and unlike the `GameManager`, the entity is only known during the iteration: the adapter would therefore have to live in `System`, not in `GameManager`. `createSystem` would have to take any callable (`template<typename... Components, typename F>`), detect the form with `std::is_invocable_v`, and wrap the form without entity in a lambda that ignores it (capturing the function **by value**). `System` would then always store the form with the entity, except for a frame system, which has no entity (`std::conditional_t`). A generic lambda, which accepts both forms, would require choosing a priority.

### Improving the view

- **The `for` syntax without allocation**: `getVector()` already provides the `for` loop, at the cost of a vector allocated on each call. A real iterator (`for (auto [e, pos, vel] : gm.view<Position, Velocity>())`) would avoid it: it would encapsulate "iterate over the smallest array and skip incomplete entities". The difficulty is that the array iterated over is chosen at runtime, among `Dense` storages of different types: the iterator would have to hold a `std::variant` of the colony iterators, or each array would have to expose a list of entities of a common type (as EnTT does). `operator*` would then return the tuple by value, which requires `auto` or `auto &&` in the loop, not `auto &`.
- **A template `each`**: `template<typename F> void each(F &&f)` would avoid the indirect `std::function` call for each entity.
- **Pairs**: `eachPair`, which yields each pair of entities only once (like `iter_combinations` in Bevy), for collisions between entities of the same group.

### Enabling and disabling an entity

Allow pausing an entity (freezing an enemy, a cutscene, a death animation…), for every system or for a single one, without removing its components. The prerequisite is met, since each `ComponentPack` knows its entity: a system can therefore check, for each entity it iterates over, whether a disabling "tag" component is present. Two points remain to be designed:

- **how to refer to a specific system**: `System<Component, Tag>` is no longer possible, since `System` is variadic and would read it as a system over two components. A distinct class will be needed, or the system will have to be referred to by its `SystemId`;
- **filtering on the absence of a component**: views and systems already filter on the *presence* of a tag, but not yet on its *absence*.

In the meantime, adding and removing a "tag" component covers most cases (see the "Pausing an entity" recipe in the user guide, section 8).
