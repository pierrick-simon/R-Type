# 02 · Why C++?

> 🌐 **Language:** [Français](../fr/02-langage.md) · English

> **Decision:** the engine and the game are written in **C++20**.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

C++ is **the best trade-off for a real-time game engine**. It is among the leaders in speed and energy, manages memory without a garbage collector (hence without unpredictable pauses), and has the richest game ecosystem (raylib, Asio, plf::colony, Unreal, Godot). Its templates allow an ECS that is typed and checked at compile time. The price to pay is **memory safety**, which the project compensates for with RAII, checked handles and sanitizers.

---

## Context

The language drives everything else: the speed of the game, the available libraries, compile times, the possible bugs. A real-time game must produce a frame every **16.7 ms** at 60 frames per second, and the server must simulate the game and answer clients at the same pace. The slightest hitch shows on screen.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Performance** | 3 | 16.7 ms budget per frame, on the client as well as on the server |
| **Game ecosystem** | 3 | Graphics, networking and container libraries; examples and documentation |
| **Memory control** | 2 | No unpredictable pauses, controlled data layout (essential for an ECS) |
| **Memory safety** | 2 | Avoid crashes and vulnerabilities caused by invalid pointers |
| **Development speed** | 2 | Compilation, iteration, readability of errors |
| **Team knowledge** | 2 | The language the team practices the most |

---

## The data

### Speed and energy: the Pereira et al. study

The most cited study on the topic compares 27 languages on the same problems (those of the *Computer Language Benchmarks Game*), measuring energy consumed, execution time and memory ([Pereira et al., SLE 2017](https://greenlab.di.uminho.pt/wp-content/uploads/2017/10/sleFinal.pdf), [DOI](https://doi.org/10.1145/3136014.3136031)). Excerpt from Table 4, normalized results (1.00 = the best; 2.00 = twice as much):

| Language | Energy | Time | Memory |
|---|---|---|---|
| C | **1.00** | **1.00** | 1.17 |
| Rust | 1.03 | 1.04 | 1.54 |
| **C++** | **1.34** | **1.56** | **1.34** |
| Java | 1.98 | 1.89 | 6.01 |
| C# | 3.14 | 3.14 | 2.85 |
| Go | 3.23 | 2.83 | 1.05 |
| Python | **75.88** | **71.90** | 2.80 |

What to take away:

- **C, Rust and C++ form the leading group.** The gaps between them (from 1 to 1.5) mostly depend on the quality of each benchmark program, not on the language itself.
- **Python is about 70 times slower than C**, and about 45 times slower than C++ (71.90 / 1.56), on these computational problems. At the scale of a game, a task that takes 0.2 ms in C++ would take about 9 ms in Python, more than half of a frame's budget.
- **Limitations:** the authors acknowledge that better programs and better compilers can change the ranking. A 2024 study disputes the methodology: according to its authors, once execution time is accounted for, the choice of language has no significant effect on energy, and the gaps mostly come from implementations (parallelism, garbage collection, JIT compilation) ([van Kempen et al., *It's Not Easy Being Green*, 2024](https://arxiv.org/html/2410.05460v3)). The Benchmarks Game team also notes that the widely shared "C++ uses 56% more energy than C" figure actually comes from the **time** column ([Benchmarks Game](https://benchmarksgame-team.pages.debian.net/benchmarksgame/energy-efficiency.html)). We therefore keep **orders of magnitude**: native compiled ≈ 1×, virtual machine ≈ 2-3×, interpreted ≈ 20-80×.

### Garbage collector pauses

C#, Java, Go and Python free memory with a **garbage collector**, which runs when it decides to. The documentation of Unity, an engine written in C++ but scripted in C#, explains it: these interruptions cause "*GC spikes*" that "*interrupt the consistent 60 fps frame rate*". Unity offers an incremental mode that does not make collection faster, but spreads it over several frames ([Unity, *Incremental garbage collection*](https://docs.unity3d.com/Manual/performance-incremental-garbage-collection.html)). In C++, memory is freed at a known point (end of scope, `destroyEntity`), with no hidden pause.

### The price of C++: memory safety

The Chromium security team (a browser written in C++) states that "*around 70% of [their] high severity security bugs are memory unsafety problems*", that is, mistakes with C/C++ pointers ([Chromium, *Memory safety*](https://www.chromium.org/Home/chromium-security/memory-safety/)). This is Rust's main argument. The risk is real, but it shrinks with discipline and tooling (see [Consequences](#decision-and-consequences)).

### C++ in the game industry

- **Godot** is written in C++ ([Godot](https://github.com/godotengine/godot)), and **Unreal Engine** gameplay is programmed in C++ ([Epic Games, *Programming with C++*](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine)).
- **Minecraft (Bedrock Edition)** uses EnTT, a C++ ECS ([EnTT](https://github.com/skypjack/entt)).
- Every library chosen by the project (raylib and raylib-cpp, Asio, plf::colony) is used directly from C++, with no binding layer.

### C++ versions

According to the 2025 JetBrains survey of 1,800 C++ developers, the most used standards are **C++20 (43%)** and **C++23 (34%)**, with C++26 at 7% ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). These are the values of the report's table: its text quotes figures shifted by one row. The project compiles as **C++20** (`CMAKE_CXX_STANDARD 20`), a complete standard well supported by GCC, Clang and MSVC. The Poc switches to C++23 with GCC 14, only for `std::stacktrace` in its error reports.

---

## The options considered

### C++ (chosen)

**Strengths**

- **Fast**, no garbage collector, full control over data layout: essential for an ECS whose whole point is to store components contiguously ([study 04](04-ecs-architecture.md)).
- **Templates:** the ECS is typed at compile time. `View<Position, Velocity>`, the duplicate check with `static_assert` or type deduction in `createSystem` have no runtime cost.
- **RAII:** a `Texture2D` or a `Window` frees its raylib resource in its destructor, with no manual call to forget.
- **The richest game ecosystem.**

**Weaknesses**

- **Memory safety** is not guaranteed by the language.
- **Slow compilation** and template error messages that are hard to read.

### C

**Strengths**

- The fastest in the table, and raylib is written in C.

**Weaknesses**

- **No templates, no RAII:** a generic ECS in C relies on `void *` and macros. The compiler no longer checks that a `Position` is read where a `Position` is expected.
- **Fewer safe abstractions**: every resource (texture, socket) is freed by hand, without a destructor.

### Rust

**Strengths**

- **As fast as C** (1.03 in energy), with **memory safety guaranteed** at compile time.
- A modern ECS engine exists: Bevy.

**Weaknesses**

- **The borrow checker fits poorly with the cross-accesses of a game** (a system that reads one entity while modifying another). The LogLog Games studio, after three years and more than 100,000 lines of Rust, left it citing slow iteration, forced refactorings and a game ecosystem that "*lives on hype*" ([LogLog Games, *Leaving Rust gamedev after 3 years*, 2024](https://loglog.games/blog/leaving-rust-gamedev/)). It is a testimony, not a measurement, but it reflects a real debate.
- **Little practiced by the team**, and the chosen libraries (raylib, Asio) are used there through unofficial bindings.

### Python

**Strengths**

- **The fastest to write**, and pygame allows quick prototyping.

**Weaknesses**

- **About 45 times slower than C++** on pure computation. For a server that simulates hundreds of entities and answers several clients 60 times per second, this rules it out.
- **The GIL** (global interpreter lock) prevents Python code from running on several cores at once. A GIL-free build exists since Python 3.13, but it remains optional: the GIL "*will remain the default*" ([PEP 703](https://peps.python.org/pep-0703/)).
- **Heavier distribution**: the interpreter must ship with the game.

### C# and Java

**Strengths**

- Productive, safe, with good tooling. C# has Unity and MonoGame.

**Weaknesses**

- **Garbage collector** (see above), and 2 to 3 times slower on average in the Pereira study.
- **The chosen libraries are in C or C++**: they would have to go through bindings, with their own maintenance.

---

## Comparison grid

| Criterion (weight) | C++ | C | Rust | Python | C# | Java |
|---|---|---|---|---|---|---|
| Performance (3) | 5 | 5 | 5 | 1 | 3 | 3 |
| Game ecosystem (3) | 5 | 3 | 3 | 3 | 4 | 2 |
| Memory control (2) | 5 | 5 | 5 | 1 | 2 | 2 |
| Memory safety (2) | 2 | 1 | 5 | 4 | 4 | 4 |
| Development speed (2) | 3 | 2 | 2 | 5 | 4 | 4 |
| Team knowledge (2) | 5 | 5 | 2 | 4 | 3 | 3 |
| **Weighted total /100** | **86** | **71** | **74** | **57** | **67** | **59** |

---

## Decision and consequences

**The project is written in C++20.** To compensate for the language's main weakness, memory safety, the project already enforces several safeguards:

- **RAII everywhere:** resources (window, textures, sounds) are freed by destructors, and the raylib window is held by a `std::unique_ptr`.
- **Handles rather than pointers:** an `Entity` is an *(index, generation)* pair. A stale handle is detected by `hasEntity` instead of referring to another entity ([design notes](../../ECS_DESIGN.en.md#entity-and-entitymanager)).
- **Stable references:** components never change address as long as they exist ([study 07](07-component-storage.md)).
- **Sanitizers:** the Poc is built by default with AddressSanitizer and UndefinedBehaviorSanitizer (`POC_SANITIZE`), which detect invalid memory accesses and undefined behavior at runtime.
- **Static analysis:** `clang-tidy` (`script/lint.sh`) runs in continuous integration.

**What we accept:** longer compile times and sometimes obscure template errors. To limit them, the public API (`GameManager`) stays simple and its type errors are documented.

> [!NOTE]
> The README shows a "C++26" badge and says GCC 14 is "required for the C++26 dialect", while the `CMakeLists.txt` files ask for C++20. The two should be aligned.

---

## When to revisit this choice

- **For a side tool** (level editor, asset generation script, network test bot), Python or C# may be more productive: C++ is only essential for the engine, the client and the server.
- **If memory bugs become the main source of crashes** despite the sanitizers, they should also be enabled on the client and server during development.
- **When GCC, Clang and MSVC fully support C++23**, upgrading will bring `std::expected`, `std::print` and `std::stacktrace` to the whole project.

---

## Sources

- Pereira et al., *Energy Efficiency across Programming Languages*, SLE 2017: <https://greenlab.di.uminho.pt/wp-content/uploads/2017/10/sleFinal.pdf> (DOI: <https://doi.org/10.1145/3136014.3136031>)
- van Kempen, Kwon, Nguyen and Berger, *It's Not Easy Being Green: On the Energy Efficiency of Programming Languages*, 2024: <https://arxiv.org/html/2410.05460v3>
- The Computer Language Benchmarks Game, on energy efficiency: <https://benchmarksgame-team.pages.debian.net/benchmarksgame/energy-efficiency.html>
- Unity, *Incremental garbage collection*: <https://docs.unity3d.com/Manual/performance-incremental-garbage-collection.html>
- Chromium, *Memory safety*: <https://www.chromium.org/Home/chromium-security/memory-safety/>
- LogLog Games, *Leaving Rust gamedev after 3 years*, April 2024: <https://loglog.games/blog/leaving-rust-gamedev/>
- PEP 703, *Making the Global Interpreter Lock Optional in CPython*: <https://peps.python.org/pep-0703/>
- JetBrains, *The State of C++ 2025*: <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- EnTT: <https://github.com/skypjack/entt>
- Godot: <https://github.com/godotengine/godot>
- Epic Games, *Programming with C++ in Unreal Engine*: <https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine>
