# R-Type

![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%E2%89%A53.25-064F8C?logo=cmake&logoColor=white)
![raylib](https://img.shields.io/badge/raylib-5.5-black?logo=raylib&logoColor=white)
![Asio](https://img.shields.io/badge/Asio-1.34.2-6A5ACD)
![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20Windows-lightgrey)
![Epitech](https://img.shields.io/badge/Epitech-2026-000000)

A multiplayer **R-Type** game written in C++20, Epitech project (2026). The
repository is organized around a reusable in-house engine (`Engine`), built on
an Entity-Component-System, and consumed by the games in `Games`: the R-Type
server and client, and a proof-of-concept program (`Poc`) that tests the ECS.

> [!NOTE]
> The project is still in progress: the engine is in place (ECS, basic
> components, rendering, audio and input), while networking and the R-Type
> gameplay are being developed.

---

## Table of contents

- [Features](#features)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Build](#build)
- [Usage](#usage)
- [Tests](#tests)
- [Documentation](#documentation)
- [Development](#development)
- [Clean](#clean)

---

## Features

| Area | What the engine provides | Library |
|---|---|---|
| **ECS** | Generational entities, components stored in sparse sets, systems run in creation order, views (`each`, `getVector`), structural changes deferred to the end of the frame | `engine-system` |
| **Basic components** | `Position2D`, `Velocity2D`, `GravityField2D`, `Friction2D`, `Clock`, `Origin2D`, `Size2D` | `engine-system` |
| **Rendering and audio** | `Window`, `Texture2D`, `Sprite2D`, `SpriteSheet2D` (animations), `Font`, `Text2D`, `Sound` | `engine-graphical` (raylib) |
| **Input** | `KeyboardManager` (AZERTY and QWERTY), `MouseManager`, `Keybind` (key rebinding) | `engine-graphical` (raylib) |
| **Networking** | UDP client and server with Asio, in-house connection and presence check (in progress) | `engine-network` |

Every component comes with its own system function (`systemXxx`), to register
with `GameManager::createSystem`. See the
[game making guide](docs/GAME_GUIDE.en.md) for a complete example.

---

## Architecture

```
R-Type/
├── Engine/                  # Reusable engine, independent from any game
│   ├── System/              # ECS (GameManager) and basic components -> `engine-system`
│   ├── Graphical/           # Window, sprites, text, sound, input (raylib) -> `engine-graphical`
│   └── Network/             # Networking (Asio) -> `engine-network`
├── Games/
│   ├── R-Type/
│   │   ├── Client/          # `r-type_client` executable
│   │   └── Server/          # `r-type_server` executable
│   └── Poc/                 # `r-type_poc` executable: ECS test suite
├── docs/                    # Guides and comparative studies (French and English)
└── script/                  # Format and lint scripts
```

- **`Engine`** doesn't depend on any game code: it exposes three reusable
  shared libraries (`engine-system`, `engine-graphical`, `engine-network`)
  consumed by `Games`, and it's the one that depends on **raylib**, **Asio**
  and **plf::colony**.
- **`Games/R-Type`** contains the client and the server, both of which rely on
  the engine, each with its own sources, headers and tests.
- **`Games/Poc`** checks the behavior of the ECS (more than 200 tests, built
  with AddressSanitizer and UndefinedBehaviorSanitizer by default).
- All executables are written directly to `Games/`.

---

## Requirements

| Tool | Version | Notes |
|---|---|---|
| **CMake** | ≥ 3.25 (tested with 4.4) | |
| **C++ compiler** | C++20 | **GCC** ≥ 14 (`g++-14`) on Linux, **MSVC** on Windows. GCC 14 also enables stack traces in the Poc |
| **Criterion** | | Unit tests, Linux only for now: `sudo apt install libcriterion-dev` |

On Linux, raylib also needs the X11 and OpenGL development packages (the same
ones as the CI):

```sh
sudo apt install libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev \
    libxcursor-dev libxinerama-dev
```

The engine's libraries are **downloaded automatically** by CMake
(`FetchContent`) on the first configuration, at pinned versions: there is
nothing to install by hand.

| Dependency | Version | Role |
|---|---|---|
| [raylib](https://github.com/raysan5/raylib) | 5.5 | Window, 2D/3D rendering, audio, input |
| [raylib-cpp](https://github.com/RobLoach/raylib-cpp) | 5.5.0 | C++ wrapper for raylib |
| [Asio](https://think-async.com/Asio/) (standalone) | 1.34.2 | UDP/TCP sockets, asynchronous I/O |
| [plf::colony](https://github.com/mattreecebentley/plf_colony) | pinned commit | Component storage with stable references |

---

## Build

```sh
# linux
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14
cmake --build build -j

# windows
cmake -S . -B build
cmake --build build --config Release
```

The first configuration downloads and builds the dependencies, which takes a
few minutes and requires an Internet connection. Reuse the same `build/`
folder afterwards.

> [!WARNING]
> Don't use `cmake .` at the root: it configures in-source and pollutes
> the repo (`CMakeCache.txt`, `CMakeFiles/`, `Makefile`, ...).

---

## Usage

```sh
./Games/r-type_server
./Games/r-type_client
./Games/r-type_poc       # runs the ECS test suite
```

---

## Tests

```sh
cmake --build build --target client_tests_run
cmake --build build --target server_tests_run
```

- The client and server tests use **Criterion**, with code coverage
  (`--coverage`). They are only built when Criterion is installed, and are
  currently disabled on Windows (see the
  [test framework study](docs/comparative-studies/en/11-test-framework.md)).
- The ECS is tested by the Poc: `./Games/r-type_poc`. To build it without the
  sanitizers, configure with `-DPOC_SANITIZE=OFF`.

---

## Documentation

Every document exists in French and English.

| Document | Content | |
|---|---|---|
| **Game making guide** | Create a game on top of the engine: folder, CMake, window, sprites, input, sound, do's and don'ts | [EN](docs/GAME_GUIDE.en.md) · [FR](docs/GAME_GUIDE.fr.md) |
| **ECS guide** | Entities, components, systems, views, the frame, errors, pitfalls, recipes, API cheat sheet | [EN](docs/ECS_GUIDE.en.md) · [FR](docs/ECS_GUIDE.fr.md) |
| **ECS design notes** | Why the ECS is built the way it is | [EN](docs/ECS_DESIGN.en.md) · [FR](docs/ECS_DESIGN.fr.md) |
| **Comparative studies** | Why Windows and Linux, C++, CMake, an ECS, raylib, Asio, `plf::colony`, sparse sets, `FetchContent`, UDP, Criterion | [EN](docs/comparative-studies/en/README.md) · [FR](docs/comparative-studies/fr/README.md) |

The engine's headers (`Engine/*/include`) are also documented with Doxygen.

---

## Development

```sh
./script/clang.sh         # check formatting (clang-format)
./script/clang.sh --fix   # fix formatting
./script/lint.sh          # check coding style (clang-tidy, uses build/compile_commands.json)
```

The CI (GitHub Actions, Ubuntu 24.04) builds the client and the server, runs
the Criterion tests and the linter on every pull request. The linter also
checks the code against C++26 (`-std=c++2c`).

---

## Clean

```sh
cmake --build build --target clean   # compiled objects only
rm -rf build                          # reconfigure everything from scratch
```
