# 08 · Sparse set or archetypes?

> 🌐 **Language:** [Français](../fr/08-stockage-ecs.md) · English

> **Decision:** the ECS stores its components as **sparse sets**: one array per component type.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

There are two main ways to store the components of an ECS. The **sparse set** (EnTT) uses one array per component type. **Archetypes** (Unity, Bevy, flecs) use one table per combination of components. Archetypes iterate faster over systems that ask for many components, but **every component added or removed moves the whole entity** from one table to another. A shoot'em up constantly creates, destroys and tags entities, and its systems rarely ask for more than three components: the sparse set suits it better. It is also much simpler to write, and compatible with stable references.

---

## Context

[Study 04](04-ecs-architecture.md) justifies the choice of an ECS, and [study 07](07-component-storage.md) the choice of container (`plf::colony`). This one answers the question in between: **how to organize components relative to each other?**

What the game does, frame after frame:

- **many structural changes**: bullets and explosions appear and disappear every frame, enemies gain or lose tags (`Stunned`, `DestroyOffScreen`…), the "Force" pod attaches and detaches;
- **systems over few components**: `Position + Velocity`, `Sprite + Texture + Position`, rarely more than three;
- **few entities**: a few hundred to a few thousand on screen, not millions.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Adding and removing components** | 3 | Creations, destructions and tags every frame |
| **Implementation simplicity** | 3 | The ECS is written and maintained by the team |
| **Iterating over several components** | 2 | This is what systems do every frame |
| **Reference stability** | 2 | A guarantee offered by the ECS guide ([study 07](07-component-storage.md)) |
| **Memory and fragmentation** | 1 | Few entities: secondary |

---

## The options considered

### Plain sparse set (chosen)

One array per component type. For a system over several components, the smallest array is iterated over, then the entity is checked for the other components.

**Strengths**

- **Adding or removing a component touches a single array.** The entity's other components do not move.
- **Simple:** one `ComponentArray` per type, and a view that performs the join.
- **Compatible with stable references**, since a component never needs to be moved ([study 07](07-component-storage.md)).

**Weaknesses**

- **The join costs one lookup per entity** and per requested component, on top of the iteration.

### Sparse set with groups (EnTT)

EnTT can group some combinations of components: the entities that have all the components of a group are stored at the start of each array, in the same order.

**Strengths**

- **Iterating over a group is as fast as a single array.** It is the best iteration speed possible ([skypjack, *ECS back and forth, part 2*](https://skypjack.github.io/2019-03-07-ecs-baf-part-2/)).

**Weaknesses**

- **A single ordering per array:** a component type can only belong to one group.
- **Elements are reordered** each time an entity enters or leaves the group, so they change address.
- **More complex**, and groups must be chosen by hand according to usage.

### Archetypes (Unity, Bevy, flecs)

One table per exact combination of components: all entities that have exactly `Position + Velocity + Sprite` live in the same table, column by column.

**Strengths**

- **Very fast iteration**: a system only visits the tables it cares about, with no check per entity.

**Weaknesses**

- **Every component added or removed moves the entity** and all its components to another table. EnTT's author puts it this way: "*every time a component is added or removed, an entity and all its components are moved from an archetype to another one*". Unity confirms it in its documentation: "*moving entities frequently is resource-intensive and reduces the performance of your application*" ([Unity Entities](https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/concepts-archetypes.html)).
- **Fragmentation:** "*the number of archetypes could explode*" when there are many combinations of components ([skypjack](https://skypjack.github.io/2019-03-07-ecs-baf-part-2/)). Every tag added creates a new table.
- **Components change address** on every move: no stable references.
- **Complex to write:** managing tables, memory chunks (16 KiB in Unity) and the graph of transitions between archetypes.

### Hybrid (Bevy)

Bevy lets the storage be chosen **per component**: table (archetype) by default, or sparse set. Its documentation sums up the trade-off: table storage is "*optimized for fast query iteration*", sparse sets "*for fast adding/removing of a component*", at the cost of slower queries ([Bevy Cheat Book](https://bevy-cheatbook.github.io/patterns/component-storage.html)).

**Strengths**

- The best of both, as needed.

**Weaknesses**

- **Two storage modes to write and maintain**, and one more choice for every component.

---

## The data

### The `ecs_benchmark` benchmark

The [ecs_benchmark](https://github.com/abeimler/ecs_benchmark) project compares several C++ ECS libraries on the same operations. Excerpt for **EnTT** (sparse set) and **flecs** (archetypes), Linux, GCC 14.2, EnTT 3.13.2, flecs 4.0.1:

| Operation | Entities | EnTT (sparse set) | flecs (archetypes) |
|---|---|---|---|
| Remove then add a component | ≈ 65,000 | **1 ms** | 15 ms |
| Remove then add a component | ≈ 1 million | **26 ms** | 250 ms |
| Create entities (2 components) | ≈ 1 million | **68 ms** | 409 ms |
| Destroy entities (2 components) | ≈ 1 million | **43 ms** | 60 ms |
| Update 2 systems | ≈ 1 million | 12 ms | 12 ms |
| Update 7 systems, mixed entities | ≈ 1 million | 40 ms | **19 ms** |

What to take away:

- **To add and remove components, the sparse set is about 10 times faster** (26 ms against 250 ms).
- **To create entities, it is about 6 times faster.**
- **Archetypes only take the lead on complex iterations** (7 systems over mixed entities), by a factor of 2.
- **Limitations:** these figures compare two complete libraries, not only their storage model. The results are not precisely dated, and they involve a million entities, far more than an R-Type. Only the **ratios** between the two approaches are kept.

### What this means for an R-Type

| Game need | Frequency | Favored approach |
|---|---|---|
| Create and destroy bullets, explosions | Several times per frame | Sparse set (≈ 6 to 10× faster) |
| Add and remove tags | Frequent | Sparse set (no entity move) |
| Systems over 1 to 3 components | Every frame | Tie (2 systems: 12 ms against 12 ms) |
| Systems over many components | Rare | Archetypes |

---

## Comparison grid

| Criterion (weight) | Plain sparse set | Sparse set + groups | Archetypes | Hybrid |
|---|---|---|---|---|
| Adding and removing components (3) | 5 | 4 | 2 | 4 |
| Implementation simplicity (3) | 5 | 2 | 2 | 1 |
| Iterating over several components (2) | 3 | 5 | 5 | 4 |
| Reference stability (2) | 5 | 2 | 1 | 3 |
| Memory and fragmentation (1) | 4 | 4 | 3 | 3 |
| **Weighted total /100** | **91** | **65** | **49** | **58** |

---

## Decision and consequences

**The ECS uses a plain sparse set**: one `ComponentArray<T>` per type (`dense` in a `plf::colony`, `sparse` in a `std::vector`), and a view that iterates over the smallest array then checks the others ([design notes](../../ECS_DESIGN.en.md#view)).

**What we accept:** one lookup in `sparse` per entity and per additional component, on every pass. It is direct indexed access (O(1)), negligible at the scale of the game.

**What it means for game code:**

- **Tags cost almost nothing**: they can be added and removed freely to mark states.
- **Putting the rarest component first in the view is not necessary**: the view picks the smallest array by itself.

---

## When to revisit this choice

- **A system iterates over tens of thousands of entities with 4 or more components**, and profiling shows the join dominates: add **groups** on that specific combination, EnTT style, rather than switching to archetypes.
- **The game almost never adds or removes components** (entities fixed after creation): archetypes would become interesting, but at the cost of stable references.

---

## Sources

- Michele Caini (skypjack), *ECS back and forth, part 2: Where are my entities?*, March 2019: <https://skypjack.github.io/2019-03-07-ecs-baf-part-2/>
- Sander Mertens, *Entity Component System FAQ*: <https://github.com/SanderMertens/ecs-faq>
- Unity, *Entities: Archetypes concepts*: <https://docs.unity3d.com/Packages/com.unity.entities@1.0/manual/concepts-archetypes.html>
- Bevy Cheat Book, *Component Storage (Table/Sparse-Set)*: <https://bevy-cheatbook.github.io/patterns/component-storage.html>
- abeimler, *ecs_benchmark*: <https://github.com/abeimler/ecs_benchmark>
