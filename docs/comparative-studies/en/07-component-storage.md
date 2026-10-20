# 07 · Why `plf::colony` to store components?

> 🌐 **Language:** [Français](../fr/07-stockage-des-composants.md) · English

> **Decision:** in the ECS, the components of a given type are stored in a **`plf::colony`**, the container `std::hive` (C++26) derives from.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

A `std::vector` is the fastest container to iterate over. But to remove an element without leaving a hole, it **moves** another element, which invalidates any reference to it. `plf::colony` guarantees that **a component never changes address as long as it exists**, with O(1) insertions and removals. This guarantee is what lets game code keep a reference to a component, keeps `getVector()` valid even when entities are destroyed, and keeps the ECS simple. The price is iteration **about 8 times slower** than a `vector` in the reference benchmark (Daniel Lemire, 2026). At the scale of an R-Type, this stays around **1% of a frame's budget**.

---

## Context

In the ECS, each component type has its `ComponentArray`, organized as a *sparse set* ([design notes](../../ECS_DESIGN.en.md#icomponentarray-and-componentarray)):

- **`dense`** holds the components themselves, which systems iterate over every frame;
- **`sparse`** links an entity to its component in `dense`, in O(1).

The choice of container for `dense` decides four things:

1. **the speed of systems**, which iterate over `dense` every frame;
2. **the speed of creations and destructions**, very frequent in a shoot'em up (bullets appear and disappear every frame);
3. **the validity of references**: can game code keep a `Position &` from one frame to the next? And does what `getVector()` returns stay valid if an entity is destroyed during the loop?
4. **the simplicity of the ECS**: what must `sparse` contain to find a component?

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Reference stability** | 3 | A guarantee offered by the ECS guide, on which `getVector()` and the references kept by the game rely |
| **Insertion and removal** | 3 | Dozens of creations and destructions per frame |
| **Iteration speed** | 2 | Every system iterates over `dense` every frame |
| **ECS simplicity** | 2 | Less synchronization code between `sparse` and `dense` means fewer bugs |
| **Memory** | 1 | A few thousand entities: secondary |
| **Longevity** | 1 | A standard container, or one on its way to becoming standard |

---

## The options considered

### `std::vector` with "swap-and-pop" (EnTT's choice)

To remove an element, it is replaced by the last one, then the last one is removed.

**Strengths**

- **The fastest possible iteration:** elements are contiguous, with no holes.
- **O(1) removal.**

**Weaknesses**

- **The last element changes address** on every removal. Any reference to it becomes invalid, and `sparse` must be fixed. Keeping a `Position &` from one frame to the next becomes impossible, and `getVector()` would be invalidated by the slightest destruction.
- **An insertion may reallocate the whole array** and invalidate *all* references at once.

EnTT, which follows this model, actually offers an optional "*pointer stability*" for the components that need it ([EnTT](https://github.com/skypjack/entt)). This shows that the need is real.

### `std::vector` with holes (*tombstones*)

The element is marked as removed without being moved, and its slot is reused later.

**Strengths**

- Elements do not move on removal.

**Weaknesses**

- **An insertion beyond capacity reallocates everything**: stability is not guaranteed, unless a maximum size is fixed in advance.
- Iteration must test every slot, and holes accumulate.

### `std::deque`

**Strengths**

- Appending at the end does not move existing elements.

**Weaknesses**

- **Removing in the middle moves elements.** We are back to the holes of the previous option.

### `std::list`

**Strengths**

- **Stable**: a node never moves.

**Weaknesses**

- **One allocation per element**: about 14 ns per insertion, against 0.8 for a `vector` (see [Data](#the-data)).
- **32 bytes per element** for an 8-byte value, because of the two pointers and the allocation. After a few removals and insertions, nodes are scattered in memory, and iteration becomes slow.

### `std::unordered_map<Entity, Component>` or `std::vector<std::unique_ptr<Component>>`

**Strengths**

- **Stable** (each element is allocated separately).

**Weaknesses**

- **Scattered in memory**: the whole point of an ECS, storing data side by side, is lost ([study 04](04-ecs-architecture.md#the-cost-of-the-cache)).

### `plf::colony` (chosen)

A chain of memory blocks of growing size. A removed element is marked in a *skipfield*, which iteration skips in O(1), and its slot is reused by a later insertion. An empty block is returned to the system ([plf::colony](https://plflib.org/colony.htm)).

**Strengths**

- **Guaranteed stability:** pointers and iterators to an element stay valid as long as that element is not itself removed, whatever insertions and removals happen elsewhere.
- **Amortized O(1) insertion and removal**, without reallocating existing elements.
- **A simpler ECS:** since an iterator stays valid, `sparse` stores **the iterator to the component directly**. There is nothing to resynchronize on removal.
- **Soon standard:** `std::hive`, derived from colony, was adopted into C++26 in February 2025 ([P3933R1](https://open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3933r1.html), [cppreference](https://cppreference.net/cpp/header/hive.html)).

**Weaknesses**

- **Slower iteration than a `vector`**: a jump in the *skipfield*, and a block change now and then.
- **No stable index shared between two containers.** The entity must therefore be stored with its component (`ComponentPack`), at least 16 more bytes per component ([design notes](../../ECS_DESIGN.en.md#icomponentarray-and-componentarray)).
- **An external dependency**, until a standard library ships `std::hive`.

---

## The data

### 1. Daniel Lemire: `std::hive` versus `vector` and `list`

Daniel Lemire, a renowned performance researcher, measured `plf::hive` (the `std::hive` implementation by colony's author) on 64-bit integers, with GCC 16.1, on an Intel Xeon Gold ([Lemire, *How fast is C++26's std::hive?*, August 2026](https://lemire.me/blog/2026/08/02/how-fast-is-c26s-stdhive/)):

| Operation (1 million elements) | `vector` | `hive` | `list` |
|---|---|---|---|
| Insertion (ns/element) | 0.81 | 1.57 | 14.22 |
| Iteration (ns/element) | **0.22** | 1.77 | 1.51 |
| Removing half, at random (ns/element) | 3.0 | **2.1** | 77.4 |
| Memory (bytes/element) | 8.4 | 9.4 | 32.0 |

His conclusion: "***The std::hive data structure is not a faster vector***". It brings stable references and cheap removals, with much less memory than a list. He also notes that at 10 million elements, removal becomes faster with a `vector` again (1.3 against 2.5 ns).

### 2. Colony's author: high-modification scenarios

The benchmarks published by colony's author confirm that `vector` "*easily*" has the best performance for pure iteration. However, in the "*high modification*" tests (5% and 10% of elements removed every frame, which resembles the bullets of a shoot'em up), **colony comes out on top** ([plflib, Haswell/GCC benchmarks](https://plflib.org/benchmarks_haswell_gcc.htm)). These are the author's own measurements: they point in the same direction as Lemire's, but should be read with that in mind.

### 3. What this means for an R-Type

An estimate based on Lemire's figures (about **1.8 ns per element** to iterate over a `hive`):

| Situation | Cost of one pass | Share of a 60 FPS frame (16.7 ms) |
|---|---|---|
| 1 system over 1,000 entities | ≈ 2 µs | ≈ 0.01% |
| 10 systems over 1,000 entities | ≈ 18 µs | ≈ 0.1% |
| 10 systems over 10,000 entities | ≈ 0.18 ms | ≈ 1% |

These are **orders of magnitude**, not measurements of the project: real components are bigger than a 64-bit integer, and a system over several components also does a lookup in `sparse` for each entity. They are enough to show that the overhead of `colony` does not matter at the scale of a shoot'em up, where the screen rarely holds more than a few thousand objects.

---

## Comparison grid

| Criterion (weight) | `vector` swap-and-pop | `vector` + holes | `deque` | `list` | `unordered_map` | `vector<unique_ptr>` | **`colony`** |
|---|---|---|---|---|---|---|---|
| Reference stability (3) | 1 | 2 | 2 | 5 | 5 | 5 | 5 |
| Insertion and removal (3) | 5 | 4 | 2 | 2 | 3 | 3 | 5 |
| Iteration speed (2) | 5 | 4 | 4 | 2 | 1 | 2 | 3 |
| ECS simplicity (2) | 3 | 3 | 3 | 4 | 5 | 3 | 5 |
| Memory (1) | 5 | 3 | 4 | 1 | 1 | 2 | 4 |
| Longevity (1) | 5 | 5 | 5 | 5 | 5 | 5 | 4 |
| **Weighted total /100** | **73** | **67** | **58** | **65** | **70** | **68** | **90** |

---

## Decision and consequences

**`dense` is a `plf::colony<ComponentPack>`**, and `sparse` a `std::vector` of iterators into `dense`. The plf::colony version is pinned to a specific commit in `Engine/CMakeLists.txt`.

**What it enables**, and what the [ECS guide](../../ECS_GUIDE.en.md) promises:

- **Keeping a reference to a component** as long as that component exists ([guide, section 3](../../ECS_GUIDE.en.md#references-to-a-component)).
- **Iterating over the result of `getVector()` while destroying entities**, even outside `update()`: the references of other entities stay valid ([guide, section 4](../../ECS_GUIDE.en.md#iterating-over-a-view-in-a-system-getvector)).
- **A short `ComponentArray`, with no resynchronization code.**

**What we accept:**

- **Iteration about 8 times slower** than a packed `vector` according to Lemire, around 1% of a frame's budget at the scale of an R-Type ([estimate](#3-what-this-means-for-an-r-type)).
- **16 more bytes per component**, to store the entity with it.
- **An external dependency** until `std::hive` ships. According to Lemire, no standard library provided it yet in August 2026.

---

## When to revisit this choice

- **When libstdc++ and MSVC provide `std::hive`**: replace `plf::colony` with `std::hive`. The principle is the same, since `std::hive` derives from colony: only a few function names change, and a dependency disappears.
- **If profiling shows that iteration dominates the frame** (tens of thousands of entities, many systems): consider a packed `vector` **only for components that do not need stable references**, as EnTT allows. Beware: this changes the guarantee offered to game code, and must be documented in the guide.

---

## Sources

- plf::colony, documentation: <https://plflib.org/colony.htm>
- plf::colony, Haswell/GCC benchmarks: <https://plflib.org/benchmarks_haswell_gcc.htm>
- Daniel Lemire, *How fast is C++26's std::hive?*, August 2026: <https://lemire.me/blog/2026/08/02/how-fast-is-c26s-stdhive/>
- WG21, P3933R1 *constexpr hive* (history of the adoption of `std::hive`): <https://open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3933r1.html>
- cppreference, `<hive>` header: <https://cppreference.net/cpp/header/hive.html>
- EnTT: <https://github.com/skypjack/entt>
