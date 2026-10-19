# 09 · How to fetch dependencies?

> 🌐 **Language:** [Français](../fr/09-gestion-des-dependances.md) · English

> **Decision:** dependencies (raylib, raylib-cpp, Asio, plf::colony) are downloaded by **`FetchContent`**, built into CMake, with pinned versions.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

`FetchContent` requires **nothing but CMake**. It works the same on Windows and Linux, fetches our four libraries and pins their versions in a single file. Its drawback is that it downloads and rebuilds everything in every new build folder. **CPM** gets the same score: it is a layer over `FetchContent` that adds a shared cache, and therefore the natural next step if configuration becomes too long. **vcpkg** is the best of the package managers (binary cache, all our libraries), but requires an installation on every machine. **Conan** does not offer raylib-cpp.

---

## Context

The engine depends on four external libraries, declared in `Engine/CMakeLists.txt`:

| Library | Version used | Type |
|---|---|---|
| raylib | `5.5` | Compiled (shared library) |
| raylib-cpp | `v5.5.0` | Header-only |
| Asio | `asio-1-34-2` | Header-only |
| plf::colony | a specific commit | Single header |

Any team member, on Windows or Linux, must be able to clone the repository and build **without installing these libraries by hand**, and everyone must build **exactly the same versions**. [Study 03](03-build-system.md) chose CMake; this one compares the ways to fetch dependencies with it.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Nothing to install** | 3 | A new machine must build with only CMake and a compiler |
| **Windows and Linux** | 3 | [Study 01](01-operating-systems.md) |
| **Our 4 libraries available** | 3 | A missing library means writing and maintaining our own recipe |
| **Pinned versions** | 2 | Everyone builds the same code, and an old build can be reproduced |
| **Repository size, ease of updates** | 2 | Do not bloat the Git repository, change a version in one line |
| **Configure time** | 2 | A new build folder must not cost long minutes |
| **No extra layer** | 1 | Fewer tools mean fewer failures |
| **Offline** | 1 | Being able to reconfigure without a connection |

---

## The data

### What C++ developers use

According to the 2025 JetBrains survey (1,800 C++ developers), most manage their libraries with **the system package manager (36%)** or **by including their sources in the build (35%)**. Dedicated managers remain a minority: **vcpkg 11%**, **NuGet 7%**, **Conan 6%**, a figure given in the report's text ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). `FetchContent` and CPM fall into the second category: the sources are downloaded then built with the project.

### Are our libraries available?

Checked in the official catalogs, on October 19, 2026:

| Library | vcpkg ([ports](https://github.com/microsoft/vcpkg/tree/master/ports)) | Conan Center ([recipes](https://github.com/conan-io/conan-center-index/tree/master/recipes)) |
|---|---|---|
| raylib | Yes (6.0) | Yes (6.0, 5.5…) |
| raylib-cpp | Yes (6.0.3) | No |
| Asio | Yes (1.32.0) | Yes (1.38.2, 1.36.0…) |
| plf::colony | Yes (7.7.15) | Yes (7.39, 7.06…) |

- **Conan Center does not offer raylib-cpp**: we would have to write and maintain our own recipe.
- **vcpkg offers all of them**, but its default versions differ from ours (raylib 6.0 instead of 5.5, Asio 1.32 instead of 1.34.2). Each version would have to be pinned by hand in the manifest.

### What each tool requires

- **vcpkg:** clone its repository, run a bootstrap script (`bootstrap-vcpkg.sh` or `.bat`), set `VCPKG_ROOT`, then pass its *toolchain* file to CMake. Its catalog has more than 2,000 libraries ([Microsoft, *Install and use packages with CMake*](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started)). Its strong point is the **binary cache, enabled by default**: an already built library is restored "*in seconds*" instead of being rebuilt ([Microsoft, *Binary Caching*](https://learn.microsoft.com/en-us/vcpkg/users/binarycaching)).
- **Conan 2:** preferably installed with `pip`, and requires **Python 3.8 or later**. Standalone installers also exist ([Conan, *Installation*](https://docs.conan.io/2/installation.html)).
- **CPM:** a single CMake file to add to the repository. It describes itself as a "*thin wrapper around CMake's FetchContent module that adds version control, caching, a simple API*". With `CPM_SOURCE_CACHE`, downloaded sources are shared between all build folders, and the project can then be configured **offline** ([CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)).
- **`FetchContent`:** built into CMake since version 3.11, nothing to install ([CMake, *FetchContent*](https://cmake.org/cmake/help/latest/module/FetchContent.html)).

### Popularity (GitHub, October 19, 2026)

| Tool | Stars | License |
|---|---|---|
| vcpkg | 27,526 | MIT |
| Conan | 9,540 | MIT |
| CPM.cmake | 4,146 | MIT |

`FetchContent` is part of CMake: it has no separate repository.

---

## The options considered

### `FetchContent` (chosen)

**Strengths**

- **Nothing to install**, identical on Windows and Linux.
- **Versions are pinned** by `GIT_TAG`, in one place (`Engine/CMakeLists.txt`). Changing a version means editing one line.
- **No recipe to write**: any Git repository will do, even one that is in no catalog.

**Weaknesses**

- **Every new build folder downloads and rebuilds everything**, raylib included.
- **A connection is needed** for the first configuration.

### CPM.cmake

**Strengths**

- **All the advantages of `FetchContent`**, plus a shared source cache and offline configuration.

**Weaknesses**

- **One more file** (`CPM.cmake`), and every developer must set `CPM_SOURCE_CACHE` to benefit from the cache.
- The cache holds **sources**: raylib is still rebuilt in every build folder.

### vcpkg (manifest mode)

**Strengths**

- **Binary cache by default**: raylib is built only once per machine.
- **All four libraries are available.**

**Weaknesses**

- **Must be installed on every machine** and in continuous integration (clone, bootstrap, environment variable, *toolchain*).
- **Versions to pin by hand** in the manifest, since they differ from the project's.

### Conan 2

**Strengths**

- Very powerful (profiles, cross-compilation, prebuilt binaries).

**Weaknesses**

- **Python or a separate installer**, on every machine.
- **raylib-cpp missing** from the catalog: one recipe to write.

### System packages (`apt`, etc.)

**Strengths**

- The simplest on Linux, and very fast (prebuilt binaries).

**Weaknesses**

- **Does not exist on Windows.**
- **Versions depend on the distribution**: two team members may build with different versions.

### Copying the sources into the repository, or Git submodules

**Strengths**

- **Offline, pinned versions, nothing to install.**

**Weaknesses**

- **Copying the sources bloats the repository**: raylib's repository, with its examples and resources, weighs several tens of megabytes. Updates are manual.
- **Submodules** avoid the copy, but require `git submodule update --init`, a step that is easy to forget.

---

## Comparison grid

| Criterion (weight) | FetchContent | CPM | vcpkg | Conan 2 | System packages | Copied sources | Submodules |
|---|---|---|---|---|---|---|---|
| Nothing to install (3) | 5 | 4 | 2 | 2 | 3 | 5 | 4 |
| Windows and Linux (3) | 5 | 5 | 5 | 5 | 1 | 5 | 5 |
| Our 4 libraries (3) | 5 | 5 | 4 | 3 | 2 | 5 | 5 |
| Pinned versions (2) | 5 | 5 | 4 | 4 | 2 | 5 | 5 |
| Repository size, updates (2) | 5 | 5 | 5 | 5 | 4 | 1 | 3 |
| Configure time (2) | 2 | 3 | 5 | 4 | 5 | 3 | 3 |
| No extra layer (1) | 5 | 4 | 3 | 2 | 5 | 5 | 4 |
| Offline (1) | 2 | 4 | 4 | 4 | 5 | 5 | 4 |
| **Weighted total /100** | **89** | **89** | **80** | **73** | **59** | **86** | **85** |

---

## Decision and consequences

**The project keeps `FetchContent`.** CPM gets the same score: both are equivalent in substance, and `FetchContent` is already in place, with no extra file or setting.

**Good practices to keep:**

- **Always pin a precise version** (`GIT_TAG` on a tag or a commit, never on a branch such as `master`), as is already the case for the four libraries.
- **Use `GIT_SHALLOW TRUE`** to download only the requested version, without the whole history (already done for Asio, raylib and raylib-cpp).
- **Reuse the same `build/` folder** rather than creating a new one, to avoid downloading everything again.

**What we accept:** a long first configuration (downloading and building raylib), and an Internet connection at that moment.

---

## When to revisit this choice

- **The team often creates new build folders** (several configurations, Debug and Release): switch to **CPM** with `CPM_SOURCE_CACHE`. The migration is light, since CPM relies on `FetchContent`.
- **Building raylib in continuous integration becomes too long**: switch to **vcpkg**, whose binary cache avoids rebuilding it on every run.
- **A dependency disappears from GitHub**: copy its sources into the repository.

---

## Sources

- JetBrains, *The State of C++ 2025*: <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- CMake, *FetchContent*: <https://cmake.org/cmake/help/latest/module/FetchContent.html>
- CMake, 3.11 release notes (introduction of `FetchContent`): <https://cmake.org/cmake/help/latest/release/3.11.html>
- CPM.cmake: <https://github.com/cpm-cmake/CPM.cmake>
- Microsoft, *Install and use packages with CMake* (vcpkg): <https://learn.microsoft.com/en-us/vcpkg/get_started/get-started>
- Microsoft, *Binary Caching* (vcpkg): <https://learn.microsoft.com/en-us/vcpkg/users/binarycaching>
- vcpkg, ports catalog: <https://github.com/microsoft/vcpkg/tree/master/ports>
- Conan, *Installation*: <https://docs.conan.io/2/installation.html>
- Conan Center Index, recipes catalog: <https://github.com/conan-io/conan-center-index/tree/master/recipes>
- GitHub API (stars and licenses), accessed October 19, 2026
