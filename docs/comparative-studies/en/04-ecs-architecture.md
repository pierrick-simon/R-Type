# 04 · Why an ECS?

> 🌐 **Language:** [Français](../fr/04-architecture-ecs.md) · English

> **Decision:** game objects are managed by an **Entity-Component-System (ECS) written by the team**, based on *sparse sets*.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

In an R-Type, objects constantly combine the same behaviors: moving, shooting, taking damage, being controlled by the keyboard, following another object. A **class hierarchy** quickly gets stuck on these combinations. An **ECS** assembles them freely: an entity is a mere identifier, its components are data, and systems apply the logic to every entity that has the right components. Data of the same type is stored side by side, which is friendly to the CPU cache, and it is easy to send over the network. The team writes its own ECS rather than using EnTT: it is tailored to the project's needs (stable references, changes deferred during a frame), with no API to learn and no dependency at the heart of the engine. Object-oriented programming is still used, but to structure the engine.

---

## Context

An R-Type is a player ship, several kinds of enemies, bullets, power-ups, explosions, scrolling backgrounds, and the famous **"Force"**, a pod that attaches to the ship, blocks bullets, shoots, and can be detached then launched again. The server must also send the state of all these objects to the clients, several dozen times per second.

The question is: **how to represent these objects in code?**

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Combining behaviors** | 3 | Gameplay keeps evolving: adding a behavior to an object must be simple |
| **Performance** | 2 | Hundreds of objects updated every frame, on the client and the server |
| **Network friendliness** | 2 | The game state must be easy to copy and send |
| **Ease of learning** | 2 | The whole team writes gameplay code |
| **Debugging, readability** | 1 | Understand what an object is doing at a given moment |
| **Control and fit to the project** | 2 | The team must understand the tool, debug it and evolve it to its needs |

---

## The options considered

### Option A: a class hierarchy

```text
GameObject (virtual update, virtual draw)
├── Ship
│   ├── PlayerShip
│   └── EnemyShip
│       └── ShootingEnemy
├── Projectile
└── PowerUp
```

**Strengths**

- **Intuitive:** this is object-oriented programming as it is usually taught.

**Weaknesses**

- **Combinations blow up the hierarchy.** Is the "Force" a `Projectile` (it is launched and deals damage), a `Ship` (it shoots) or a `PowerUp` (it is picked up)? It would take multiple inheritance, with the diamond problem, or duplicated code. This is the classic problem Robert Nystrom describes in *Game Programming Patterns* ([*Component* chapter](https://gameprogrammingpatterns.com/component.html)).
- **Base classes grow:** everything shared "moves up" into `GameObject`, which ends up knowing how to do everything.
- **Objects scattered in memory:** each object is allocated separately and reached through a pointer, which costs a cache miss per object.

### Option B: object composition (Unity's "GameObject" model)

A `GameObject` holds a list of components, each with its own virtual `update()` method.

**Strengths**

- **Solves the combinations:** the "Force" has a `Mover`, a `Weapon` and a `Damage`.

**Weaknesses**

- **Logic stays inside components**, and components that need to talk to each other (a `Weapon` that reads the `Mover`'s position) have to look each other up.
- **Still scattered objects**, and one virtual call per component and per frame.

### Option C: an ECS (chosen)

```text
Entity    = an identifier (index + generation)
Component = plain data: Position, Velocity, Health, Weapon, Player...
System    = a function: "for every entity with Position and Velocity, move forward"
```

**Strengths**

- **Combinations are free.** The "Force" is `Position + Velocity + Hitbox + Weapon + AttachedTo`. To detach it, just remove `AttachedTo`. No class to create.
- **Contiguous data:** all `Position` components are stored together. A system that iterates over them reads memory in order, the most favorable case for the CPU cache (see [Data](#the-data)).
- **Network friendly:** an entity's state is its components, plain data structs with no pointers and no logic. Copying them into a packet is straightforward.
- **Clean separation:** data (components) does not know about logic (systems). A behavior is added without touching the rest.

**Weaknesses**

- **A different way of thinking:** "what does this entity have?" instead of "what is this object?". The [ECS guide](../../ECS_GUIDE.en.md) and its documented pitfalls are there to help with this transition.
- **Scattered logic:** an enemy's behavior is spread over several systems. One must read the order of systems to understand it.

### Option D: an existing ECS (EnTT, flecs)

**Strengths**

- **Mature, optimized and proven:** EnTT is used by Minecraft (Bedrock Edition) ([EnTT](https://github.com/skypjack/entt)), and flecs offers entity relationships, a query language and an explorer ([flecs](https://github.com/SanderMertens/flecs)).

**Weaknesses**

- **An external dependency at the heart of the engine:** at the slightest unexpected behavior, one must dive into a large codebase, and its guarantees (validity of references, changes during an iteration) are not the ones the project wants to offer its game code.
- **A rich API, hence long to learn**, for features an R-Type does not need.

The project's ECS actually reuses EnTT's ideas (*sparse sets*, views, generational handles): the [design document](../../ECS_DESIGN.en.md) details what was reused and what was done differently.

---

## The data

### The cost of the cache

Reading a byte from main memory can cost **hundreds of CPU cycles**. When the processor reads memory, it loads a whole **cache line** (usually 64 bytes): if the next data is right next to it, it is already there. Robert Nystrom measured the effect by replacing objects reached through pointers with contiguous arrays of components: **the update loop became fifty times faster**. He notes that the gain varies between machines ([*Game Programming Patterns*, *Data Locality* chapter](https://gameprogrammingpatterns.com/data-locality.html)).

This figure is a favorable case. At the scale of an R-Type (a few hundred to a few thousand entities), performance is not the main argument for the ECS. **Flexibility** and **network friendliness** are what matter. Performance is a bonus that leaves headroom for the server.

### Who uses an ECS?

- **Overwatch** (Blizzard): its gameplay architecture and netcode rely on an ECS, presented at GDC 2017. According to the talk, the team uses it to "*curtail complexity*" while adding new features ([GDC Vault, *Overwatch Gameplay Architecture and Netcode*](https://gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and)).
- **Minecraft Bedrock**: uses EnTT ([EnTT](https://github.com/skypjack/entt)).
- **Unity DOTS** and **Bevy** (Rust): engines built around an ECS ([ECS FAQ](https://github.com/SanderMertens/ecs-faq)).

### The criticism

An ECS is not always the right tool. The LogLog Games studio, after three years of development, considers that many games do not need one, and that ECS is sometimes adopted out of hype more than necessity ([LogLog Games](https://loglog.games/blog/leaving-rust-gamedev/)). The argument holds for a small single-player game with fixed gameplay. It weighs less here: the game is **multiplayer**, its state must be **serialized**, and the engine must be able to host several games.

### Sparse set or archetypes?

*Summary below; the full comparison, with benchmarks, is in [study 08](08-ecs-storage.md).*

There are two main ways to store components ([ECS FAQ, Sander Mertens](https://github.com/SanderMertens/ecs-faq)):

| | **Sparse set** (EnTT, this project) | **Archetypes** (Unity DOTS, Bevy, flecs) |
|---|---|---|
| **Principle** | One array per component type | One table per combination of components |
| **Adding or removing a component** | Fast: a single array is modified | Expensive: the entity changes table, all its data is moved |
| **Iterating over several components** | Iterate over the smallest array and look up the others | Very fast: everything is aligned in the same table |
| **Implementation complexity** | Low | High |

An R-Type adds and removes many components (bullets created and destroyed every frame, tags added and removed), and its systems rarely involve more than three components. The **sparse set** is therefore the best trade-off, and by far the simplest to write.

---

## Comparison grid

| Criterion (weight) | A. Inheritance | B. Object composition | C. In-house ECS | D. EnTT / flecs |
|---|---|---|---|---|
| Combining behaviors (3) | 1 | 4 | 5 | 5 |
| Performance (2) | 2 | 2 | 4 | 5 |
| Network friendliness (2) | 2 | 3 | 5 | 5 |
| Ease of learning (2) | 5 | 4 | 3 | 2 |
| Debugging, readability (1) | 4 | 4 | 3 | 3 |
| Control and fit to the project (2) | 4 | 4 | 5 | 2 |
| **Weighted total /100** | **55** | **70** | **87** | **77** |

---

## Decision and consequences

**Game objects are managed by an in-house, sparse set based ECS.** Its API fits in a single class, `RType::GameManager`, documented in the [ECS guide](../../ECS_GUIDE.en.md).

**Object-oriented programming is not abandoned, it moves.** It structures the **engine**: a facade (`GameManager`), interfaces (`IComponentArray`, `ISystem`) and template classes implementing them, encapsulation (`EntityManager`, `ComponentManager` and `SystemManager` are internal details). The ECS organizes the **game data**. The two approaches complement each other.

**What we accept:**

- **A learning curve**, offset by the guide, its documented pitfalls and the 200+ tests of the Poc.
- **An object's logic is spread over several systems.** The order of systems must be documented and thought through ([game making guide](../../GAME_GUIDE.en.md#5-the-order-of-systems)).
- **An engine to maintain:** every feature (views, frame systems, `getVector()`) is written and tested by the team.

---

## When to revisit this choice

- **Systems often involve 4 or more components, over thousands of entities**, and profiling shows the join is expensive: consider groups (like EnTT) or archetypes.
- **The ECS becomes the team's bottleneck** (missing features, engine bugs): compare the maintenance cost with a library such as EnTT, and migrate if it offers the same guarantees.

---

## Sources

- Robert Nystrom, *Game Programming Patterns*, *Component* chapter: <https://gameprogrammingpatterns.com/component.html>
- Robert Nystrom, *Game Programming Patterns*, *Data Locality* chapter: <https://gameprogrammingpatterns.com/data-locality.html>
- Sander Mertens, *Entity Component System FAQ*: <https://github.com/SanderMertens/ecs-faq>
- Timothy Ford, *Overwatch Gameplay Architecture and Netcode*, GDC 2017: <https://gdcvault.com/play/1024001/-Overwatch-Gameplay-Architecture-and>
- EnTT: <https://github.com/skypjack/entt>
- flecs: <https://github.com/SanderMertens/flecs>
- LogLog Games, *Leaving Rust gamedev after 3 years*: <https://loglog.games/blog/leaving-rust-gamedev/>
