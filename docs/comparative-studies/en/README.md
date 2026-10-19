# Comparative studies

> 🌐 **Language:** [Français](../fr/README.md) · English

This folder gathers the **comparative studies** behind the technical choices of the R-Type project. Each study answers a question such as "why X rather than Y?": it compares the possible options against explicit criteria, relies on sourced figures, then presents the decision, its cost, and the conditions that could call it into question.

- **Audience:** the team, and anyone who joins the project or is interested in its technical choices.
- **To use the engine**, see [the game making guide](../../GAME_GUIDE.en.md) and [the ECS guide](../../ECS_GUIDE.en.md) instead.

---

## The studies

| No. | Question | Choice | Main alternatives |
|---|---|---|---|
| 01 | [Why Windows and Linux?](01-operating-systems.md) | Windows + Linux | macOS, consoles, mobile, Linux only |
| 02 | [Why C++?](02-language.md) | C++20 | C, Rust, Python, C#, Java |
| 03 | [Why CMake?](03-build-system.md) | CMake + `FetchContent` | Make, Meson, Bazel, Premake, Visual Studio projects |
| 04 | [Why an ECS?](04-ecs-architecture.md) | In-house ECS (sparse set) | Object inheritance, object composition (Unity-style), existing ECS (EnTT, flecs) |
| 05 | [Why raylib?](05-graphics-library.md) | raylib 5.5 + raylib-cpp | SFML, SDL3, GLFW + OpenGL, full engine |
| 06 | [Why Asio for networking?](06-network-library.md) | Standalone Asio (no Boost) | Native sockets, Boost.Asio, ENet, GameNetworkingSockets, SFML Network |
| 07 | [Why `plf::colony` to store components?](07-component-storage.md) | `plf::colony` | `std::vector`, `std::deque`, `std::list`, `std::unordered_map`, `std::vector<std::unique_ptr>` |
| 08 | [Sparse set or archetypes?](08-ecs-storage.md) | Plain sparse set | Sparse set with groups (EnTT), archetypes (Unity, Bevy, flecs), hybrid |
| 09 | [How to fetch dependencies?](09-dependency-management.md) | `FetchContent` | CPM, vcpkg, Conan, system packages, vendored sources, submodules |
| 10 | [UDP or TCP, with Asio?](10-network-protocol.md) | UDP, with an in-house connection | TCP only, TCP and UDP together |
| 11 | [Which test framework?](11-test-framework.md) | Criterion | GoogleTest, Catch2, doctest, Boost.Test, in-house runner |

## The method

Each study follows the same outline, inspired by *Architecture Decision Records* (ADR), a format proposed by Michael Nygard to document a decision together with its context and consequences ([source](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)):

1. **In short:** the decision and its main reason, in three lines.
2. **Context:** the problem to solve and the project's constraints.
3. **Criteria:** what matters for this choice, weighted from 1 (secondary) to 3 (essential).
4. **Options considered:** every serious alternative, with its strengths and weaknesses.
5. **Data:** benchmarks, surveys and official documentation, always sourced.
6. **Comparison grid:** a score from 1 to 5 per criterion and per option, then a weighted total.
7. **Decision and consequences:** what was chosen, what we gain, what we accept to lose.
8. **When to revisit this choice:** the signals that would justify reconsidering the decision.
9. **Sources.**

### Reading the comparison grids

The scores (1 = poor, 5 = excellent) are **the team's assessment**, not a measurement. Each one is justified in the text of the study. The weighted total is the sum of (score × weight), scaled to 100. It helps summarize, but does not replace reading the study: a single eliminating criterion (for instance "does not work on Windows") can be enough to rule out an option.

### Reading the figures

A benchmark measures one specific case, on one specific machine. The studies always state **what was measured**, **by whom** and **under which conditions**, and draw orders of magnitude from it rather than absolute truths. When a source is disputed (as is the energy ranking of programming languages, for instance), the study says so.

---

## Constraints shared by all the studies

A few project needs come up in every study:

- **A real-time multiplayer game:** a server that is authoritative over the game, and clients that display it. At 60 frames per second, each frame has a budget of **16.7 ms**, on the client as well as on the server.
- **Two target systems:** Windows and Linux ([study 01](01-operating-systems.md)).
- **An in-house, reusable engine:** the `Engine/` folder (ECS, rendering, networking) is independent from the game, and the `Games/` folder can host several games. The project therefore owns its game loop, its object model and its networking, rather than delegating them to an existing engine.
- **A small team:** every tool must be quick to learn, and the project must build without a complicated setup.

> [!NOTE]
> These studies reflect the state of the repository on October 19, 2026. Networking (`engine-network`) is implemented on the `feature/network` branch, not yet merged into `dev`: studies 06 and 10 describe that implementation.
