# 03 · Why CMake?

> 🌐 **Language:** [Français](../fr/03-systeme-de-build.md) · English

> **Decision:** the project is built with **CMake**, and its dependencies are downloaded by **`FetchContent`**.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

A `Makefile` does not work as is with the Windows compiler (MSVC), and cannot download dependencies. **CMake** describes the project once, then generates Makefiles or Ninja files on Linux, and a Visual Studio solution on Windows. It is **the most used** C++ build tool (59 to 79% of developers depending on the survey). `FetchContent` downloads raylib, Asio and plf::colony at configure time, with a pinned version: CMake and a compiler are all it takes to build everything.

---

## Context

The project has three libraries (`engine-system`, `engine-graphical`, `engine-network`), three executables (client, server, Poc), tests, and four external dependencies (raylib, raylib-cpp, Asio, plf::colony). It must build on **Linux (GCC, Clang)** and on **Windows (MSVC)** ([study 01](01-operating-systems.md)).

A build tool must therefore:

1. **compile** the sources in the right order, rebuilding only what changed;
2. **work on both systems**, with their respective compilers;
3. **fetch the dependencies**, without asking every team member to install them by hand.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Cross-platform (including MSVC)** | 3 | A single build description for Windows and Linux |
| **Dependency management** | 3 | raylib, Asio and plf::colony must arrive without manual installation |
| **Adoption, documentation, IDEs** | 2 | Online help, integration in CLion, Visual Studio, VS Code |
| **Simplicity** | 2 | Everyone must be able to add a file or a target |
| **Build speed** | 1 | Fast incremental builds |

---

## The data

### What C++ developers use

**JetBrains 2025 survey** (1,800 C++ developers), build tools used regularly ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)):

| Tool | Share |
|---|---|
| **CMake** | **59%** |
| Visual Studio projects (MSBuild) | 34% |
| Makefiles | 27% |
| Ninja | 15% |

**ISO C++ 2021 survey** (1,853 answers, several choices allowed) ([Standard C++ Foundation](https://isocpp.org/files/papers/CppDevSurvey-2021-04-summary.pdf)):

| Tool | Share |
|---|---|
| **CMake** | **79.38%** |
| Make / nmake | 41.23% |
| MSBuild | 38.96% |
| Ninja | 33.35% |
| Meson | 4.48% |
| Bazel | 3.78% |

The two surveys do not ask the same people, hence the gap, but they say the same thing: **CMake is the de facto standard**, far ahead of Meson or Bazel.

### What CMake does, exactly

CMake compiles nothing itself: it is a **generator**. In its documentation's words, "*a CMake Generator is responsible for writing the input files for a native build system*" ([CMake, *cmake-generators*](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)). From the same `CMakeLists.txt` files, it produces:

- **Makefiles** or **Ninja** files on Linux;
- a **Visual Studio solution** (MSBuild) on Windows;
- an **Xcode** project on macOS.

Make and CMake are therefore not really competitors: on Linux, `cmake --build build` runs `make`. CMake adds the layer that makes the project portable.

### Dependencies

According to the same JetBrains 2025 survey, most C++ developers manage their libraries with the **system package manager (36%)** or by **including their sources in the build (35%)**. Dedicated managers remain a minority: **vcpkg 11%**, **Conan 6%** (figure given in the report's text) ([JetBrains](https://lp.jetbrains.com/the-state-of-cpp-2025/)).

---

## The options considered

### CMake + `FetchContent` (chosen)

**Strengths**

- **A single description for Windows and Linux**, with the most widespread tool.
- **`FetchContent` downloads the dependencies at configure time** and builds them with the project. Versions are pinned in `Engine/CMakeLists.txt`: `asio-1-34-2`, raylib `5.5`, raylib-cpp `v5.5.0`, and a specific commit for plf::colony. Everyone builds exactly the same code.
- **Nothing to install** besides CMake and a compiler: no Python (Conan), no tool to bootstrap (vcpkg).
- **Native integration** in CLion, Visual Studio and VS Code. `compile_commands.json` (enabled by `CMAKE_EXPORT_COMPILE_COMMANDS`) feeds `clang-tidy` and code completion.

**Weaknesses**

- **The CMake language is verbose** and has evolved a lot: an old tutorial may teach bad practices (global variables rather than targets).
- **The first configuration downloads and builds raylib**, which takes time and an Internet connection. A new build folder starts the download again.

### Make alone

**Strengths**

- Known by the whole team, and simple for a small project.

**Weaknesses**

- **Does not work with MSVC as is.** GNU Make does not ship with Windows, and Microsoft's equivalent (`nmake`) has a different syntax. Two build files would have to be maintained.
- **No dependency management:** raylib would have to be installed by hand on every machine, or custom download scripts written.
- **File dependencies to write yourself** (`-MMD`) to rebuild only what is needed.

### Meson

**Strengths**

- Clearer syntax than CMake, fast builds (it relies on Ninja), built-in dependency system (*wraps*).

**Weaknesses**

- **Not widespread** (4.48% in the ISO 2021 survey), and requires Python.

### Bazel

**Strengths**

- Reproducible and very fast builds on huge repositories (designed by Google).

**Weaknesses**

- **Heavy to set up** for a project of this size, and not widespread in C++ (3.78%).

### Premake, Visual Studio projects

**Weaknesses**

- **Premake:** pleasant Lua syntax, but not widespread and without dependency management.
- **Visual Studio projects alone:** Windows only.

### And for dependencies: Conan, vcpkg, CPM?

*Summary below; the full comparison is in [study 09](09-dependency-management.md).*

| Option | Advantage | Drawback |
|---|---|---|
| **`FetchContent` (chosen)** | Built into CMake, nothing to install | Rebuilds the dependencies in every build folder |
| **CPM.cmake** | Layer over `FetchContent`, with a shared cache (`CPM_SOURCE_CACHE`) | One more file in the repository |
| **vcpkg** | Huge catalog, cached binaries | Must be cloned and bootstrapped; 11% adoption |
| **Conan** | Powerful, handles cross-compilation | Requires Python and a separate installation; 6% adoption |
| **System packages** (`apt`) | The simplest on Linux | Does not exist on Windows, versions differ between distributions |

CPM describes itself as a layer over `FetchContent` ([CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)), so both rely on the same mechanism. Switching from one to the other is easy if configure time becomes a problem.

---

## Comparison grid

| Criterion (weight) | CMake | Make | Meson | Bazel | Premake | Visual Studio |
|---|---|---|---|---|---|---|
| Cross-platform (3) | 5 | 2 | 5 | 4 | 4 | 1 |
| Dependencies (3) | 4 | 1 | 4 | 4 | 1 | 3 |
| Adoption, IDEs (2) | 5 | 3 | 2 | 2 | 2 | 4 |
| Simplicity (2) | 3 | 3 | 4 | 1 | 4 | 3 |
| Build speed (1) | 4 | 3 | 5 | 5 | 3 | 4 |
| **Weighted total /100** | **85** | **44** | **80** | **64** | **55** | **55** |

---

## Decision and consequences

**The project is built with CMake, and its dependencies arrive through `FetchContent`.** In practice:

- **Configure:** `cmake -S . -B build`, always in a separate folder. `cmake .` at the root pollutes the repository (see the [README](../../../README.md)).
- **Add a game:** a folder under `Games/`, its `CMakeLists.txt` and an `add_subdirectory` line ([game making guide](../../GAME_GUIDE.en.md#2-creating-the-game-folder)).
- **Change a dependency's version:** edit its `GIT_TAG` in `Engine/CMakeLists.txt`, a single place.

**What we accept:** a longer first configuration (downloading and building raylib) and an Internet connection at that moment.

> [!NOTE]
> The README asks for "CMake ≥ 4.4", but the `CMakeLists.txt` files declare `cmake_minimum_required(VERSION 3.25...4.0)` and continuous integration installs `>=3.25`. These three values should be aligned.

---

## When to revisit this choice

- **Configuration becomes too long** for the team: switch to CPM with a shared cache, or to vcpkg in manifest mode (cached binaries).
- **A dependency is no longer available** on GitHub (repository deleted or renamed): copy its sources into the repository, or switch to a package manager.
- **The project adds a platform** (web, mobile): check that the dependency in question builds there with `FetchContent`.

---

## Sources

- JetBrains, *The State of C++ 2025*: <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- Standard C++ Foundation, *2021 Annual C++ Developer Survey "Lite"*: <https://isocpp.org/files/papers/CppDevSurvey-2021-04-summary.pdf>
- CMake, *cmake-generators(7)*: <https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>
- CMake, *FetchContent*: <https://cmake.org/cmake/help/latest/module/FetchContent.html>
- CPM.cmake: <https://github.com/cpm-cmake/CPM.cmake>
