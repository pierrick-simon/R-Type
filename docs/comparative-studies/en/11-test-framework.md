# 11 · Which test framework?

> 🌐 **Language:** [Français](../fr/11-framework-de-tests.md) · English

> **Decision:** the unit tests of the client, the server and the engine use **Criterion**, to be enabled on Windows too.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

The project uses **Criterion**: it is already integrated into the build and continuous integration, known by the team, and it runs each test in its own process, which makes it possible to detect a crash cleanly. It works on Windows (MinGW and Visual Studio 2015+), where its authors test it continuously. One step remains: our `CMakeLists.txt` disables it on Windows, and it must be built there with Meson, since there is no Windows binary and no vcpkg or Conan package. **GoogleTest**, the most widespread C++ framework, is the alternative if this installation becomes a burden.

---

## Context

Current state of the repository:

- **The client and the server** each have a Criterion test target (`client_tests_run`, `server_tests_run`), with code coverage (`--coverage`).
- **The root `CMakeLists.txt`** looks for a Criterion installation already present on the machine (`find_library`), and **only outside Windows** (`if (NOT WIN32 ...)`).
- **Continuous integration** installs Criterion with `apt-get install libcriterion-dev`.
- **The Poc** (`Games/Poc`) has its own test runner, with more than 200 dependency-free tests. It is a validation program for the ECS: it is not affected by this choice.

> [!NOTE]
> The README states that Criterion is "*automatically downloaded if missing*". This is not the case: CMake only looks for it on the machine.

Tests will become essential with networking ([study 10](10-network-protocol.md)): reading and writing headers, connection, presence checks. This code must be tested **on Windows as well as on Linux**, because that is where compiler differences (type sizes, byte order) show up.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Team knowledge** | 3 | A tool the team already masters allows writing tests right away |
| **Crash isolation** | 2 | The engine handles memory and networking: a crashing test must not hide the others |
| **Already in place** | 2 | Build, code coverage and continuous integration are already configured |
| **Windows (MSVC) and Linux** | 2 | The project's two targets ([study 01](01-operating-systems.md)) |
| **Installation and CMake integration** | 1 | Fetched without manual installation ([study 09](09-dependency-management.md)) |
| **Adoption and documentation** | 1 | Examples, answers, IDE integration |
| **Features** | 1 | Fixtures, parameterized tests, *mocks* |
| **Compile time** | 1 | Tests are rebuilt often |

---

## The data

### What C++ developers use

JetBrains' *The State of C++ 2025* report is based on the answers of 1,800 C++ developers from 22 countries, taken from its *Developer Ecosystem Survey 2025*. JetBrains notes that the results may be slightly skewed toward users of its products ([JetBrains, *The State of C++ 2025*](https://lp.jetbrains.com/the-state-of-cpp-2025/)). Unit testing frameworks used:

| Framework | Share of C++ developers |
|---|---|
| **GoogleTest** | **31%** |
| CppUnit | 8% |
| Boost.Test | 7% |
| Catch2 | 7% |
| CppUTest | 5% |
| doctest | 4% |
| Other | 2% |
| Tests written without a framework | 17% |
| No unit tests in C++ | 37% |

Criterion does not appear in the list: it is less widespread, but this criterion weighs little for a team that already knows it.

### Popularity and activity (GitHub, October 19, 2026)

| Framework | Stars | License | Latest release |
|---|---|---|---|
| GoogleTest | 39,655 | BSD-3-Clause | v1.18.0 (August 2026) |
| Catch2 | 21,536 | BSL-1.0 | v3.16.1 (October 2026) |
| doctest | 6,900 | MIT | v2.5.3 (July 2026) |
| Criterion | 2,308 | MIT | v2.5.0 (October 2026) |

All four are actively maintained: Criterion published its latest release this month.

### Criterion

- **Cross-platform:** Criterion "*runs on Linux, FreeBSD, macOS, and Windows (compiling with MinGW GCC and Visual Studio 2015+)*", and is used from C as well as C++ ([Criterion](https://github.com/Snaipe/Criterion)).
- **Tested on Windows:** its continuous integration builds and tests the library on `windows-latest`, in addition to Linux and macOS ([Criterion, `ci.yml`](https://github.com/Snaipe/Criterion/blob/bleeding/.github/workflows/ci.yml)).
- **Isolation:** each test runs in its own process. A crash is reported as the failure of that test, without stopping the others, and can even be tested on purpose ([Criterion](https://github.com/Snaipe/Criterion)).
- **Installation on Windows:** published releases only provide a binary for Linux x86-64 ([Criterion, *Releases*](https://github.com/Snaipe/Criterion/releases)), and Criterion is neither in vcpkg nor in Conan Center (checked on October 19, 2026). It must therefore be built, with **Meson, Ninja, pkg-config, libffi and libgit2** ([Criterion, *Setup*](https://criterion.readthedocs.io/en/master/setup.html)). To do so, Criterion's own continuous integration installs Ninja, CMake and Python with Chocolatey.

### The other frameworks

- **GoogleTest:** its official documentation shows integration through `FetchContent`, then `gtest_discover_tests` so that `ctest` discovers each test. It requires **at least C++17** ([GoogleTest, *Quickstart: CMake*](https://google.github.io/googletest/quickstart-cmake.html)). It also provides GoogleMock (mocks) and crash tests (*death tests*).
- **Catch2 v3:** is no longer a single header, but "*a normal library, with multiple headers and separately compiled implementation*". It targets C++14 and later, and offers sections, BDD-style macros and micro-benchmarks ([Catch2](https://github.com/catchorg/Catch2)).
- **doctest:** a single header, which presents itself as "*by far the fastest both in compile times*" among feature-rich frameworks ([doctest](https://github.com/doctest/doctest)). Its benchmarks (GCC, 2018) report for instance **11 s against 99 s** to include the header in 200 files, compared with Catch 2. Catch2 v3, now compiled separately, is not included ([doctest, *Benchmarks*](https://github.com/doctest/doctest/blob/master/doc/markdown/benchmarks.md)).

---

## The options considered

### Criterion (chosen)

**Strengths**

- **Already in place:** test targets, code coverage and continuous integration are configured.
- **Known by the team:** simple syntax (`Test(suite, name)`, `cr_assert`), usable right away.
- **Test isolation** in separate processes: a crash is reported cleanly, without hiding the other tests.
- **Works on Windows**, and is tested there by its authors.

**Weaknesses**

- **On Windows, it must be built** with Meson and its dependencies, on every machine and in continuous integration.
- **Cannot be fetched with `FetchContent`**, since it is not built with CMake.
- **Not widespread** in C++ (absent from the JetBrains survey): less help and less IDE integration.

### GoogleTest

**Strengths**

- **By far the most used**, with the best integration in IDEs (CLion, Visual Studio, VS Code) and in `ctest`.
- **`FetchContent` and MSVC**, officially documented: nothing to install.
- **The most complete:** fixtures, parameterized tests, GoogleMock, crash tests.

**Weaknesses**

- **A migration:** targets, continuous integration and syntax to change, and a new tool for the team.
- **No systematic crash isolation:** *death tests* must be written to test a crash.

### Catch2 v3

**Strengths**

- **Very readable syntax** (sections, `REQUIRE(a == b)` showing the values), `FetchContent`, MSVC.

**Weaknesses**

- **A migration**, and less widespread than GoogleTest (7% against 31%).

### doctest

**Strengths**

- **The fastest to compile**, a single header.

**Weaknesses**

- **A migration**, not widespread (4%), fewer advanced features.

### Boost.Test

**Weaknesses**

- **Depends on Boost**, which the project deliberately avoided ([study 06](06-network-library.md)).

### An in-house runner (like the Poc)

**Strengths**

- No dependency, works everywhere.

**Weaknesses**

- **Everything must be written:** reports, filters, `ctest` and IDE integration, fixtures. Enough for the Poc, not for the whole project.

---

## Comparison grid

| Criterion (weight) | Criterion | GoogleTest | Catch2 v3 | doctest | Boost.Test | In-house runner |
|---|---|---|---|---|---|---|
| Team knowledge (3) | 5 | 3 | 2 | 2 | 2 | 4 |
| Crash isolation (2) | 5 | 4 | 2 | 2 | 2 | 1 |
| Already in place (2) | 5 | 2 | 2 | 2 | 2 | 2 |
| Windows (MSVC) and Linux (2) | 4 | 5 | 5 | 5 | 4 | 5 |
| Installation and CMake integration (1) | 2 | 5 | 5 | 5 | 2 | 5 |
| Adoption, documentation (1) | 1 | 5 | 3 | 2 | 3 | 1 |
| Features (1) | 3 | 5 | 4 | 3 | 4 | 1 |
| Compile time (1) | 4 | 3 | 3 | 5 | 3 | 5 |
| **Weighted total /100** | **82** | **75** | **60** | **60** | **52** | **62** |

---

## Decision and consequences

**The project keeps Criterion.** To cover Windows as well:

1. **Remove the `NOT WIN32` condition** from the Criterion block of the root `CMakeLists.txt`, so that tests are looked for and built on Windows too.
2. **Document the installation on Windows:** build Criterion with Meson and Ninja, then install it where `find_library` will find it (or give its path to CMake).
3. **Add a Windows build to continuous integration**, which builds Criterion the way its own continuous integration does, then runs the tests.
4. **Fix the README**, which announces an automatic download of Criterion that does not exist.

**What we accept:** a heavier installation on Windows than for a framework fetchable with `FetchContent`, and a framework less widespread than GoogleTest.

**The Poc keeps its in-house runner:** it is a demonstration and validation program for the ECS, self-contained by design.

---

## When to revisit this choice

- **Installing Criterion on Windows becomes a burden** for the team or for continuous integration: switch to **GoogleTest**, fetched with `FetchContent`. It then only takes replacing `Test(...)` with `TEST(...)` and `cr_assert` with `EXPECT_TRUE`, and calling `gtest_discover_tests`.
- **Tests need mocks** (for instance a fake socket to test networking without a connection): GoogleMock, shipped with GoogleTest, becomes a real argument.
- **Criterion arrives in vcpkg or Conan, or publishes Windows binaries**: installing it becomes as simple as the others.

---

## Sources

- JetBrains, *The State of C++ 2025*: <https://lp.jetbrains.com/the-state-of-cpp-2025/>
- Criterion: <https://github.com/Snaipe/Criterion>
- Criterion, continuous integration (`ci.yml`): <https://github.com/Snaipe/Criterion/blob/bleeding/.github/workflows/ci.yml>
- Criterion, releases: <https://github.com/Snaipe/Criterion/releases>
- Criterion, *Setup*: <https://criterion.readthedocs.io/en/master/setup.html>
- GoogleTest, *Quickstart: Building with CMake*: <https://google.github.io/googletest/quickstart-cmake.html>
- Catch2: <https://github.com/catchorg/Catch2>
- doctest: <https://github.com/doctest/doctest>
- doctest, *Benchmarks*: <https://github.com/doctest/doctest/blob/master/doc/markdown/benchmarks.md>
- vcpkg, ports catalog: <https://github.com/microsoft/vcpkg/tree/master/ports>
- Conan Center Index, recipes catalog: <https://github.com/conan-io/conan-center-index/tree/master/recipes>
- GitHub API (stars, licenses and latest releases), accessed October 19, 2026
