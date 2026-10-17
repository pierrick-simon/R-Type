# ECS User Guide

> 🌐 **Language:** [Français](ECS_GUIDE.fr.md) · English

This guide explains how to **use** the engine's ECS (`Engine/System`) day to day: creating entities, attaching components to them, writing systems and, above all, **avoiding the most common pitfalls**.

- **Audience:** anyone writing game code on top of the engine. No prior knowledge of ECS is required.
- **To understand the architectural choices** (the *why*), see [the ECS design notes](ECS_DESIGN.en.md).

The whole API goes through a single class: **`RType::GameManager`**. The internal classes (`EntityManager`, `ComponentManager`, `SystemManager`) are implementation details: game code must never use them directly.

> [!TIP]
> Every rule in this guide is covered by a test in the Poc project (`Games/Poc`). If you are unsure about a behavior, run the `r-type_poc` binary or read the matching test.

---

## Table of contents

1. [The essentials in 30 seconds](#1-the-essentials-in-30-seconds)
2. [Entities](#2-entities)
3. [Components](#3-components)
4. [Systems](#4-systems)
5. [The frame: immediate and deferred operations](#5-the-frame-immediate-and-deferred-operations)
6. [Error handling](#6-error-handling)
7. [Pitfalls to avoid](#7-pitfalls-to-avoid)
8. [Recipes](#8-recipes)
9. [API cheat sheet](#9-api-cheat-sheet)

### Glossary

| Term | Definition |
|---|---|
| **Entity** (`Entity`) | An identifier that represents a game object (ship, bullet, enemy…). It holds no data. |
| **Component** | A data struct attached to an entity (position, health…). An entity has at most one component of each type. |
| **System** | A function called on every `update()`: once per entity that has all the requested components, or only once if no component is requested (frame system). This is where game logic lives. |
| **View** | An object that iterates over the entities having a set of components, and provides for each one the entity and its components. |
| **Frame** | One call to `gm.update()`: every system runs once, in order. |
| **Handle** | The `Entity` value you keep in order to refer to an entity later. |

---

## 1. The essentials in 30 seconds

```cpp
#include "GameManager.hpp"

struct Body { float x; float y; float vx; float vy; };
struct Health { int hp; };

RType::GameManager gm;

RType::Entity ship = gm.createEntity();
gm.addComponent<Body>(ship, 10.f, 50.f, 1.f, 0.f);
gm.addComponent<Health>(ship, 3);

gm.createSystem<Body>([](Body &b) {
    b.x += b.vx;
    b.y += b.vy;
});

while (running) {
    gm.update();
}
```

Three ideas to remember:

- **An entity is just an identifier.** It contains nothing: its components carry the data.
- **A component is a plain data struct.** No inheritance, no logic.
- **A system is a function called on every entity that has the requested components**, once per `update()`.

---

## 2. Entities

### Create, destroy, check

```cpp
RType::Entity e = gm.createEntity();   // always immediate
gm.destroyEntity(e);                   // destroys the entity AND all of its components
bool alive = gm.hasEntity(e);          // false once the entity is destroyed
```

### Generational handles

When an entity is destroyed, its slot (its *index*) is **recycled** for a future entity. So that an old handle can never refer to the new entity, every `Entity` also carries a **generation**, incremented each time the slot is recycled:

```cpp
RType::Entity enemy = gm.createEntity();   // index 3, generation 0
gm.destroyEntity(enemy);

RType::Entity bullet = gm.createEntity();  // index 3, generation 1

gm.hasEntity(enemy);             // false: stale generation
gm.hasComponent<Health>(enemy);  // false
gm.getComponent<Health>(enemy);  // throws NoSuchEntity
enemy == bullet;                 // false: same index, different generation
```

A stale handle is therefore **harmless**: it will never touch the entity that took over its slot. This is what makes it safe to store the `Entity` of **another** entity (a target, a child…) inside a component.

### What the `Entity` type supports

| Operation | Available |
|---|---|
| Copy, compare with `==` | Yes |
| `e.index()`, `e.generation()` | Yes (for debugging, or to index your own array) |
| Default constructor `Entity e;` | **No**: an `Entity` always comes from `createEntity()` |
| `std::set<Entity>`, `std::map<Entity, …>` | **No** (no `operator<`) |
| `std::unordered_map<Entity, …>` | **No** (no `std::hash`) |

To keep a set of entities, a `std::vector<Entity>` combined with `std::ranges::find` / `std::erase` is enough.

> [!WARNING]
> **Never build an `Entity` by hand** (`RType::Entity(3)`, `RType::Entity(3, 2)`). The constructor is public for technical reasons, but a hand-made handle can refer to an entity that is not yours as soon as its generation happens to match. An `Entity` is only obtained through `createEntity()`, or from a view.

---

## 3. Components

### Writing a component

A component is a struct of **plain data**:

```cpp
struct Health {
    int hp;
};

struct Player {};   // "tag" component: empty, used only as a marker

struct Target {
    RType::Entity entity;   // ANOTHER entity: store its handle
};
```

- A component must be **movable**. Non-copyable types (holding a `std::unique_ptr`, for example) are accepted.
- A component **does not need to know its own entity**: when a system needs it, a view provides it (see [section 4](#accessing-the-entity-from-a-system)). Only store an `Entity` to refer to *another* entity.

### Add, read, remove

```cpp
gm.addComponent<Position>(e, 10.f, 20.f);  // arguments are forwarded to the constructor
gm.addComponent<Health>(e);                // no argument: every field set to zero
gm.addComponent<Player>(e);                // a tag

Position &pos = gm.getComponent<Position>(e);
pos.x += 1.f;                              // modifies the stored component directly

if (gm.hasComponent<Player>(e)) { /* ... */ }

gm.removeComponent<Health>(e);             // removes one component
gm.removeComponents(e);                    // removes ALL components; the entity stays alive
```

- `addComponent` **returns nothing**. If you need the component right after, use `getComponent` (outside `update()`, adding is immediate: see [section 5](#5-the-frame-immediate-and-deferred-operations)).
- Arguments are forwarded to the constructor **with parentheses**, which also works with plain structs: `Position{10.f, 20.f}` and `addComponent<Position>(e, 10.f, 20.f)` give the same result.
- An entity has **at most one component of each type**. Adding a `Health` to an entity that already has one throws `AlreadyExistingComponent`.

### Iterating over entities: views

To iterate over the entities that have one or several component types, use a **view**:

```cpp
gm.view<Position, Velocity>().each(
    [](RType::Entity e, Position &pos, const Velocity &vel) {
        pos.x += vel.vx;
        pos.y += vel.vy;
    });
```

- The function is called for every entity that has **all** the requested types. It receives **the entity first**, then a reference to each component, in the order of the types. A parameter can be `const` if the component is only read.
- A view on a type that has **never been added** is simply empty: it never throws.
- The view automatically iterates over the type that has **the fewest components**: the order of the types does not affect performance.
- A view copies nothing and keeps no list of entities: creating one costs almost nothing. Create one when you need it, rather than keeping it around.
- The same type cannot appear twice: `gm.view<Position, Position>()` does not compile.
- Outside `update()`, do not destroy entities during an `each` (see [section 7, pitfall 3](#pitfall-3-destroying-entities-during-an-iteration-outside-update)).

> [!NOTE]
> The iteration order **does not match the insertion order**: freed slots are reused. Never rely on any particular order.

To count the components of a type: `gm.getArray<T>().nbComponent()`. `getArray<T>()` also gives low-level access to the storage (`getDense()`, whose elements expose `.component` and `.entity()`), but throws `NoSuchComponent` if no `T` has ever been added: to iterate, always prefer a view.

### References to a component

A reference obtained with `getComponent` (or during an iteration) **stays valid when other components are added or removed**. It only becomes invalid when **that specific component** is removed (or its entity destroyed).

```cpp
Position &pos = gm.getComponent<Position>(player);
for (int i = 0; i < 1000; i++)
    gm.addComponent<Position>(gm.createEntity(), 0.f, 0.f);
pos.x = 5.f;                         // OK: pos is still valid

gm.destroyEntity(player);
pos.x = 6.f;                         // FORBIDDEN: the component no longer exists
```

---

## 4. Systems

### Creating a system

```cpp
RType::SystemId gravity = gm.createSystem<Velocity>([](Velocity &v) {
    v.y += 0.5f;
});
```

- The component type **must be specified** between angle brackets: `createSystem<Velocity>(...)`.
- On every `gm.update()`, the function is called **once for each existing `Velocity`**.
- Systems run **in creation order**. A system sees the changes made by earlier systems during the same frame.
- As long as one of the requested types has never been added, the system simply does nothing, without error.

### A system over several components

A system can request several component types. The function then receives a reference to each of them, in the same order, and is only called for the entities that have **all** of them:

```cpp
gm.createSystem<Position, Velocity>([](Position &pos, const Velocity &vel) {
    pos.x += vel.vx;
    pos.y += vel.vy;
});
```

- An entity that only has some of the types is skipped, without error.
- A parameter can be `const` if the system only reads that component.
- The same type cannot appear twice: `createSystem<Position, Position>` does not compile (*each component type must appear only once*).
- The order of the types does not affect performance: the system automatically iterates over the type that has **the fewest components**.

### Receiving the `GameManager`

A system that has to act on the world (create or destroy entities, read another component…) **receives the `GameManager` as its first parameter**:

```cpp
gm.createSystem<Homing, Position>(
    [](RType::GameManager &gm, Homing &h, Position &pos) {
        if (!gm.hasEntity(h.target))
            return;
        const Position &goal = gm.getComponent<Position>(h.target);
        pos.x += (goal.x - pos.x) * 0.1f;
    });
```

Both forms, with and without the `GameManager`, can be mixed freely. Do not capture the `GameManager` in a system's lambda (`[&gm]`): receiving it as a parameter lets you write the system as a **plain function**, in a separate file (see below). Inside a system, on the other hand, the lambda passed to `each` may capture the received `gm`: it only lives for the duration of the iteration.

### A frame system

A system created **without any component type** is called **only once per `update()`**. It receives the `GameManager`, or nothing:

```cpp
gm.createSystem([](RType::GameManager &gm) {   // same as: gm.createSystem<>(...)
    gm.view<Enemy>().each([](RType::Entity, Enemy &) { /* ... */ });
});
```

It is the natural place for anything that does not concern a single entity: **collisions** between two groups of entities (see the [recipe](#collisions-between-bullets-and-enemies)), enemy waves spawning, the score, a timer…

A frame system behaves like the others: it has a `SystemId`, runs at its rank in creation order, and its structural changes are deferred to the end of the frame.

### Accessing the entity from a system

The function of a per-entity system receives the components, **not the entity**. When a system needs the entity (to destroy it, add or remove a component…), write a **frame system that iterates over a view**: the view provides the entity.

```cpp
struct Weapon {
    int ammo;   // no need to store the entity
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Weapon, Position>().each(
        [&gm](RType::Entity ship, Weapon &w, const Position &from) {
            RType::Entity bullet = gm.createEntity();
            gm.addComponent<Position>(bullet, from.x, from.y);
            gm.addComponent<Velocity>(bullet, 5.f, 0.f);
            if (--w.ammo <= 0)
                gm.removeComponent<Weapon>(ship);   // out of ammo: remove the weapon
        });
});
```

Do not copy the entity into a field of the component (`owner`, `self`): it would have to be filled in by hand on every `addComponent`, and a mistake would go unnoticed.

### Writing systems in files

A system can be a lambda, but also a **plain function**, which lets you group the game logic in dedicated files:

```cpp
// Systems.hpp
inline void move(Position &pos, const Velocity &vel)
{
    pos.x += vel.vx;
    pos.y += vel.vy;
}

inline void collisions(RType::GameManager &gm)
{
    gm.view<Bullet, Position>().each(/* ... */);
}

// At startup
gm.createSystem<Position, Velocity>(move);
gm.createSystem(collisions);
```

A function **defined** in a header must be `inline`; otherwise, declare it in the `.hpp` and define it in a `.cpp`.

When several pieces of data always go together, group them into a single component (like `Body { x, y, vx, vy }` in the opening example): one component, one system, no lookup.

### Destroying a system

```cpp
gm.destroySystem(gravity);
gm.hasSystem(gravity);   // false
```

A `SystemId` is **never reused**: an old id can never destroy another system by mistake. Destroying a system that no longer exists has no effect.

> [!IMPORTANT]
> **Create your systems once, at startup, and keep them.** To turn a behavior on or off, use data in components (a `bool enabled`, a tag you add or remove) rather than creating and destroying systems mid-game. See [section 7](#7-pitfalls-to-avoid).

---

## 5. The frame: immediate and deferred operations

This is **the** key concept of this ECS.

During an `update()`, systems iterate over component arrays. If a system destroyed an entity mid-iteration, it would corrupt the array being read. To prevent this, **structural changes requested during `update()` are queued** in a command queue, then applied **all together at the end of the frame**, in the order they were requested.

Outside `update()`, everything is immediate.

| Operation | Outside `update()` | During `update()` (inside a system) |
|---|---|---|
| `createEntity` | Immediate | Immediate |
| `destroyEntity` | Immediate | **End of frame** |
| `addComponent` | Immediate | **End of frame** |
| `removeComponent` | Immediate | **End of frame** |
| `removeComponents` | Immediate | **End of frame** |
| `createSystem` | Immediate | Immediate (not supported: see [section 7](#7-pitfalls-to-avoid)) |
| `destroySystem` | Immediate | **End of frame** |
| `getComponent`, `hasComponent`, `hasEntity`, `hasSystem`, `view` | Read | Reads the state **before** pending commands are applied |

### What this means in practice

```cpp
struct Life {
    int hp;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Life>().each([&gm](RType::Entity e, Life &l) {
        if (l.hp > 0)
            return;
        gm.destroyEntity(e);

        gm.hasEntity(e);                 // still TRUE: the destruction is pending
        gm.getComponent<Position>(e);    // still works until the end of the frame
    });
});
```

- An entity destroyed during the frame **remains visible and readable until the end of the frame**. Later systems still see it.
- A component added during the frame **only appears at the end of it**: `hasComponent` returns `false` until then, and systems and views only visit it starting from the next frame.
- An entity created during the frame exists right away (`createEntity` is immediate), but its components are only added at the end of the frame.
- If two systems destroy the same entity in the same frame (two bullets hit the same enemy), there is no danger: the second destruction has no effect.
- An entity destroyed during the frame is **not** recycled within the same frame: `createEntity` will never reuse its slot before the frame ends.
- Calling `gm.update()` from a system has no effect (no nested frames).

---

## 6. Error handling

Every ECS exception derives from `RType::EngineException` (declared in `Exceptions.hpp`).

| Call | Situation | Result |
|---|---|---|
| `destroyEntity(e)` | `e` already destroyed or stale | No effect |
| `hasComponent<T>(e)` | `e` destroyed or stale | `false` (never throws) |
| `getComponent<T>(e)` | `e` destroyed or stale | `NoSuchEntity` |
| | `e` has no `T` | `NoSuchComponent` |
| `addComponent<T>(e, ...)` | `e` destroyed or stale | `NoSuchEntity` |
| | `e` already has a `T` | `AlreadyExistingComponent` |
| | `T`'s constructor throws | the constructor's exception, **immediately** |
| `removeComponent<T>(e)` | `e` destroyed or stale | `NoSuchEntity` |
| | `e` has no `T` | `NoSuchComponent` |
| `removeComponents(e)` | `e` destroyed or stale | `NoSuchEntity` |
| `view<Ts...>()` | one of the `Ts` has never existed | Empty view (never throws) |
| `getArray<T>()` | no `T` has ever existed | `NoSuchComponent` |
| `destroySystem(id)` | system already destroyed | No effect |

### Errors during `update()`

During `update()`, a pending command only runs **at the end of the frame**. Its error therefore does **not** come out of the call made inside the system, but out of `gm.update()`:

```cpp
struct Bullet {
    RType::Entity target;
};

gm.createSystem<Bullet>([](RType::GameManager &gm, Bullet &b) {
    try {
        gm.addComponent<Damage>(b.target, 1);   // b.target may already be destroyed...
    } catch (const RType::NoSuchEntity &) {
        // ... but this catch will NEVER be reached
    }
});

try {
    gm.update();                                // NoSuchEntity is thrown HERE
} catch (const RType::EngineException &e) {
    std::cerr << e.what() << std::endl;
}
```

The right approach is to **check before acting** rather than catch the error afterwards:

```cpp
gm.createSystem<Bullet>([](RType::GameManager &gm, Bullet &b) {
    if (gm.hasEntity(b.target) && !gm.hasComponent<Damage>(b.target))
        gm.addComponent<Damage>(b.target, 1);
});
```

### What `update()` guarantees when something throws

- **The frame is always applied.** Even if a system throws, every command already requested during the frame is executed before the exception leaves `update()`.
- **A failing command does not prevent the following ones.** Every command in the queue is executed.
- **Only one error is propagated.** If a system threw, its exception is the one that leaves `update()` (command errors are then ignored). Otherwise, it is the error of the first failing command. Other errors are lost: hence the value of checking before acting.
- **The ECS remains usable afterwards.** The next `update()` works normally; nothing is replayed.

---

## 7. Pitfalls to avoid

Each pitfall shows the code **not to write**, followed by the **recommended** solution.

### Pitfall 1: calling the `GameManager` from a component's constructor or destructor

**Don't:**

```cpp
struct Link {
    RType::GameManager *gm;
    RType::Entity child;
    ~Link() { gm->destroyEntity(child); }   // FORBIDDEN
};
```

A component's destructor is called **while** the ECS is applying a command (`destroyEntity`, `removeComponent`, end of frame). Calling back into the `GameManager` at that point causes a **stack overflow** or memory corruption.

**Do:** perform the operation in a system.

```cpp
struct Parent {
    RType::Entity child;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Parent, Health>().each(
        [&gm](RType::Entity self, Parent &p, const Health &h) {
            if (h.hp > 0)
                return;
            gm.destroyEntity(p.child);   // the parent dies: the child is destroyed with it
            gm.destroyEntity(self);
        });
});
```

A component's constructor or destructor must only manage **its own resources** (memory, texture or sound handle…).

### Pitfall 2: creating a system during `update()`

`createSystem` is immediate, even inside a system. The new system is appended to the list **while that list is being iterated**, so it runs in the current frame. A system that creates another one on every pass makes `update()` loop forever.

**Do:** create all your systems at startup. For a temporary behavior, enable it through data in a component.

### Pitfall 3: destroying entities during an iteration outside `update()`

Outside `update()`, `destroyEntity` is **immediate**: it removes components from the array being iterated over.

**Don't:**

```cpp
gm.view<Lifetime>().each([&gm](RType::Entity e, Lifetime &l) {
    if (l.ticks <= 0)
        gm.destroyEntity(e);                // FORBIDDEN: the iteration is corrupted
});
```

**Do:** do it in a system, where destruction is automatically deferred (see the [recipe](#a-limited-lifetime-bullets-particles-explosions)). If you really have to do it outside `update()`, collect first, destroy afterwards:

```cpp
std::vector<RType::Entity> expired;
gm.view<Lifetime>().each([&expired](RType::Entity e, const Lifetime &l) {
    if (l.ticks <= 0)
        expired.push_back(e);
});
for (RType::Entity e : expired)
    gm.destroyEntity(e);
```

### Pitfall 4: keeping a pointer or reference to another entity's component

**Don't:**

```cpp
struct Homing {
    Position *target;   // FORBIDDEN: dangling as soon as the target is destroyed
};
```

**Do:** keep the entity's **handle**, and check that it exists before every access.

```cpp
struct Homing {
    RType::Entity target;
};

gm.createSystem<Homing>([](RType::GameManager &gm, Homing &h) {
    if (!gm.hasEntity(h.target))
        return;   // the target is destroyed (or its slot recycled): stop following it
    Position &goal = gm.getComponent<Position>(h.target);
    // ...
});
```

Thanks to generations, `hasEntity` stays reliable even if the target's slot has been reused by another entity.

### Pitfall 5: capturing short-lived variables by reference

A system's function is **kept for as long as the system exists**. Anything it captures by reference must live at least that long.

**Don't:**

```cpp
void setupScore(RType::GameManager &gm)
{
    int score = 0;
    gm.createSystem<Enemy>([&score](Enemy &) { score++; });
}   // score is destroyed here: the system then writes to freed memory
```

**Do:** store the data in a component, or capture an object that lives as long as the `GameManager` (a member of your `Game` class, for example). The `GameManager` itself never needs to be captured: receive it as a parameter.

### Pitfall 6: relying on `try`/`catch` around a call inside a system

See [section 6](#errors-during-update): during `update()`, the error of an `addComponent`, `removeComponent` or `removeComponents` comes out of `gm.update()`, not out of the call itself.

**Do:** check with `hasEntity` / `hasComponent` before acting.

### Pitfall 7: reading a component right after adding it, inside a system

**Don't:**

```cpp
gm.createSystem<Spawner>([](RType::GameManager &gm, Spawner &s) {
    RType::Entity e = gm.createEntity();
    gm.addComponent<Health>(e, 10);
    gm.getComponent<Health>(e);   // throws NoSuchComponent: the addition is pending
});
```

**Do:** keep the value in a local variable if you need it right away. The component will exist at the end of the frame.

---

## 8. Recipes

### A typical game loop

```cpp
// Systems.hpp: one system = one function
inline void readInput(Input &in) { /* ... */ }
inline void move(Body &b) { /* ... */ }
inline void collisions(RType::GameManager &gm) { /* views over bullets and enemies */ }
inline void killDead(RType::GameManager &gm) { /* view over Health: destroyEntity if hp <= 0 */ }
inline void draw(const Body &b, const Sprite &s) { /* ... */ }

// main.cpp
RType::GameManager gm;

// 1. At startup: every system, in the desired execution order.
gm.createSystem<Input>(readInput);
gm.createSystem<Body>(move);
gm.createSystem(collisions);
gm.createSystem(killDead);
gm.createSystem<Body, Sprite>(draw);

// 2. The initial entities.
spawnPlayer(gm);

// 3. The main loop.
while (running) {
    gm.update();
}
```

### A limited lifetime (bullets, particles, explosions)

```cpp
struct Lifetime {
    int ticks;
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Lifetime>().each([&gm](RType::Entity e, Lifetime &l) {
        if (--l.ticks <= 0)
            gm.destroyEntity(e);   // deferred: safe during iteration
    });
});

RType::Entity bullet = gm.createEntity();
gm.addComponent<Lifetime>(bullet, 60);
```

### Pausing an entity without destroying a system

```cpp
struct Frozen {};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Body>().each([&gm](RType::Entity e, Body &b) {
        if (gm.hasComponent<Frozen>(e))
            return;
        b.x += b.vx;
        b.y += b.vy;
    });
});

gm.addComponent<Frozen>(enemy);      // pause
gm.removeComponent<Frozen>(enemy);   // resume
```

### A projectile aiming at a target that may be destroyed

```cpp
struct Missile {
    RType::Entity target;   // ANOTHER entity: store its handle
};

gm.createSystem([](RType::GameManager &gm) {
    gm.view<Missile, Position>().each(
        [&gm](RType::Entity self, Missile &m, Position &pos) {
            if (!gm.hasEntity(m.target)) {
                gm.destroyEntity(self);   // no target left: the missile self-destructs
                return;
            }
            const Position &goal = gm.getComponent<Position>(m.target);
            pos.x += (goal.x - pos.x) * 0.1f;
            pos.y += (goal.y - pos.y) * 0.1f;
        });
});
```

### Collisions between bullets and enemies

A frame system, with one view nested in another: each bullet is tested against each enemy.

```cpp
inline void collisions(RType::GameManager &gm)
{
    gm.view<Bullet, Position, Damage>().each(
        [&gm](RType::Entity bullet, Bullet &, const Position &bp, const Damage &d) {
            gm.view<Enemy, Position, Health>().each(
                [&](RType::Entity, Enemy &, const Position &ep, Health &h) {
                    if (!overlap(bp, ep))
                        return;
                    h.hp -= d.value;
                    gm.destroyEntity(bullet);   // deferred: the bullet stays visible until the end of the frame
                });
        });
}

gm.createSystem(collisions);
```

- A bullet that hits two enemies in the same frame damages both of them, since its destruction is deferred. If that is not intended, record in the bullet that it has already hit.
- The cost is proportional to *bullets × enemies*. Beyond a few hundred entities, a spatial grid built once per frame, at the start of the system, becomes necessary.

---

## 9. API cheat sheet

```cpp
namespace RType {

class GameManager {
public:
    void update();

    // Entities
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;

    // Components
    template<typename Component, typename... Args>
    void addComponent(Entity entity, Args &&...args);
    template<typename Component>
    void removeComponent(Entity entity);
    void removeComponents(Entity entity);
    template<typename Component>
    Component &getComponent(Entity entity);
    template<typename Component>
    bool hasComponent(Entity entity) const;
    template<typename... Components>
    View<Components...> view();              // .each([](Entity, Components &...) {...})
    template<typename Component>
    ComponentArray<Component> &getArray();   // .nbComponent()

    // Systems (no type: once per frame)
    template<typename... Components>
    SystemId createSystem(std::function<void(Components &...)> func);
    template<typename... Components>
    SystemId createSystem(std::function<void(GameManager &, Components &...)> func);
    void destroySystem(SystemId id);
    bool hasSystem(SystemId id) const;
};

}
```

| Goal | Call |
|---|---|
| Create an entity | `gm.createEntity()` |
| Check whether an entity still exists | `gm.hasEntity(e)` |
| Destroy an entity and its components | `gm.destroyEntity(e)` |
| Add a component | `gm.addComponent<T>(e, args...)` |
| Read or modify a component | `gm.getComponent<T>(e)` |
| Check whether an entity has a component | `gm.hasComponent<T>(e)` |
| Remove a component | `gm.removeComponent<T>(e)` |
| Empty an entity without destroying it | `gm.removeComponents(e)` |
| Iterate over entities, with their entity | `gm.view<T, ...>().each(func)` |
| Count the components of a type | `gm.getArray<T>().nbComponent()` |
| Add a per-entity behavior | `gm.createSystem<T, ...>(func)` (at startup) |
| Add a global processing, once per frame | `gm.createSystem(func)` (at startup) |
| Give the `GameManager` to a system | `GameManager &` as the first parameter of `func` |
| Remove a behavior | `gm.destroySystem(id)` |
| Advance the game by one frame | `gm.update()` |
