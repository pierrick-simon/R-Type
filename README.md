# R-Type

[![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-%E2%89%A53.25-064F8C?logo=cmake&logoColor=white)](https://cmake.org/)
[![raylib](https://img.shields.io/badge/raylib-5.5-black?logo=raylib&logoColor=white)](https://github.com/raysan5/raylib)
[![Asio](https://img.shields.io/badge/Asio-1.34.2-6A5ACD)](https://think-async.com/Asio/)
[![Lua](https://img.shields.io/badge/Lua-5.4-2C2D72?logo=lua&logoColor=white)](https://www.lua.org/)
[![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20Windows-lightgrey)](#requirements)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Epitech](https://img.shields.io/badge/Epitech-2026-000000)](https://www.epitech.eu/)

A multiplayer **R-Type** game written in C++20, Epitech project (2026). The
repository is organized around a reusable in-house engine (`Engine`), built on
an Entity-Component-System, and consumed by the games in `Games`: the R-Type
server and client.

> [!NOTE]
> The project is still in progress: the engine (ECS, rendering, audio, input,
> networking, Lua runner) and the client/server architecture are in place,
> while the R-Type gameplay is being extended.

---

## Table of contents

- [Features](#features)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Build](#build)
- [Install](#install)
- [Usage](#usage)
- [Tests](#tests)
- [Documentation](#documentation)
- [Development](#development)
- [Contributing](#contributing)
- [Clean](#clean)
- [Troubleshooting](#troubleshooting)
- [Authors](#authors)
- [License](#license)

---

## Features

| Area | What the engine provides | Library |
|---|---|---|
| **ECS** | Generational entities, components stored in sparse sets, systems run in creation order, views (`each`, `getVector`), structural changes deferred to the end of the frame | `engine-system` |
| **Basic components** | `Position2D`, `Velocity2D`, `GravityField2D`, `Friction2D`, `Clock`, `Origin2D`, `Size2D` | `engine-system` |
| **2D rendering** | `Window`, `Texture2D`, `Sprite2D`, `SpriteSheet2D` (animations), `Font`, `Text2D` | `engine-graphical2D` (raylib) |
| **3D rendering** | Camera, models, 3D sprites | `engine-graphical3D` (raylib) |
| **Input** | `KeyboardManager` (AZERTY and QWERTY), `MouseManager`, `Keybind` (key rebinding) | `engine-graphical2D` (raylib) |
| **Audio** | `Sound`, `Listener` | `engine-audio` (miniaudio) |
| **Networking** | UDP client and server with Asio, in-house connection and presence check | `engine-network` |
| **Lua scripting** | `game` executable that runs a game written in Lua from a `config.conf` | `game` (Lua 5.4, sol2) |

Components with a behavior come with their own system function (`systemXxx`),
to register with `GameManager::createSystem`; pure data components
(`Position2D`, `Origin2D`, `Size2D`, `Texture2D`, `Font`) have none. See the
[game making guide](docs/GAME_GUIDE.en.md) for a complete example.

---

## Architecture

```
R-Type/
├── Engine/                  # Reusable engine, independent from any game
│   ├── System/              # ECS (GameManager) and basic components -> `engine-system`
│   ├── Graphical2D/         # Window, sprites, text, input (raylib) -> `engine-graphical2D`
│   ├── Graphical3D/         # Camera, models, 3D sprites (raylib) -> `engine-graphical3D`
│   ├── Audio/               # Sounds (miniaudio) -> `engine-audio`
│   ├── Network/             # Networking (Asio) -> `engine-network`
│   └── Game/                # Lua game runner (sol2) -> `game` executable
├── Games/
│   └── R-Type/
│       ├── Shared/          # Network protocol and assets shared by client and server
│       ├── Client/          # `r-type_client` executable
│       ├── Server/          # `r-type_server` executable
│       └── sprites/         # Game assets, loaded relative to `Games/`
├── tests/
│   └── engine/              # ECS unit tests (Criterion) -> `engine_tests_run`
├── docs/                    # Guides and comparative studies (French and English)
└── script/                  # Format and lint scripts
```

- **`Engine`** doesn't depend on any game code: it exposes reusable shared
  libraries (`engine-system`, `engine-graphical2D`, `engine-graphical3D`,
  `engine-audio`, `engine-network`) consumed by `Games`, and it's the one that
  depends on **raylib**, **Asio**, **miniaudio**, **Lua**, **sol2** and
  **plf::colony**.
- **`Games/R-Type`** contains the client and the server, both of which rely on
  the engine, each with its own sources, headers and tests.
- **`tests/engine`** checks the behavior of the ECS (more than 200 unit tests,
  built with AddressSanitizer and UndefinedBehaviorSanitizer by default).
- All executables are written directly to `Games/`.

---

## Requirements

Only the tools below have to be installed by hand. Everything else is
downloaded by CMake (see [the table further down](#libraries-downloaded-by-cmake)).

| Tool | Version | Needed for | Notes |
|---|---|---|---|
| **Git** | any | Configuring | CMake clones the libraries with Git on the first configuration. |
| **CMake** | ≥ 3.25 (tested with 4.4) | Building | Ubuntu 24.04's `cmake` (3.28) is enough. |
| **C++ compiler** | C++20 | Building | **GCC ≥ 13** on Linux (tested with GCC 13, 14 and 16; the default `g++` of Ubuntu 24.04 and Fedora is enough). **MSVC** (Visual Studio 2022, "Desktop development with C++") on Windows. |
| **X11 and OpenGL headers** | | Building, Linux only | Needed by raylib to open the window. |
| **Criterion** | any (tested with 2.4.1) | Tests, Linux only (not Fedora) | Without it, the test targets aren't created. |
| **AddressSanitizer** (`libasan8`) | same as GCC | Engine tests, Linux only | Detects memory errors (out-of-bounds access, use-after-free, ...). The engine tests are built with `-fsanitize=address` by default. |
| **UndefinedBehaviorSanitizer** (`libubsan1`) | same as GCC | Engine tests, Linux only | Detects undefined behavior (signed overflow, null or misaligned pointer, ...). The engine tests are built with `-fsanitize=undefined` by default. Both sanitizers are usually installed with `g++`; turn them off with `-DENGINE_TESTS_SANITIZE=OFF`. |
| **clang-format / clang-tidy** | 22 | Contributing only | `script/clang.sh` calls `clang-format-22`, `script/lint.sh` calls `clang-tidy` (version 22, like the CI), see [Development](#development). |

On Ubuntu / Debian, everything needed to build and test is installed with:

```sh
sudo apt install git cmake g++ gcc libasan8 libubsan1 libcriterion-dev \
    libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev
```

On Fedora, everything needed to build is installed with:

```sh
sudo dnf install git cmake gcc-c++ gcc libasan libubsan \
    mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel
```

> [!NOTE]
> The tests don't work on Fedora: Criterion isn't packaged there, so the
> test targets aren't created. The game itself builds normally.

### Libraries downloaded by CMake

These are **downloaded automatically** by CMake (`FetchContent`) on the first
configuration, at the pinned versions below: there is nothing to install by
hand. Each one is looked for on the system first and only downloaded if it's
missing: raylib must be ≥ 5.5 and Lua 5.4, while for the header-only libraries
(raylib-cpp, Asio, miniaudio, sol2, plf::colony) the installed copy is used
whatever its version.

| Dependency | Version | Role |
|---|---|---|
| [raylib](https://github.com/raysan5/raylib) | 5.5 | Window, 2D/3D rendering, input |
| [raylib-cpp](https://github.com/RobLoach/raylib-cpp) | 5.5.0 | C++ wrapper for raylib |
| [Asio](https://think-async.com/Asio/) (standalone) | 1.34.2 | UDP/TCP sockets, asynchronous I/O |
| [miniaudio](https://github.com/mackron/miniaudio) | 0.11.22 | Sound playback (`engine-audio`) |
| [Lua](https://www.lua.org/) | 5.4.7 | Scripting language of the `game` runner |
| [sol2](https://github.com/ThePhD/sol2) | 3.5.0 | C++ ↔ Lua bindings |
| [plf::colony](https://github.com/mattreecebentley/plf_colony) | pinned commit | Component storage with stable references |

To forbid any download and only use libraries already on the system, configure
with `-DRTYPE_FETCH_MISSING_DEPS=OFF`.

---

## Build

```sh
# linux
cmake -S . -B build
cmake --build build -j

# windows
cmake -S . -B build
cmake --build build --config Release
```

If the default `g++` is older than GCC 13, install a newer one and pass it to
CMake, for example `-DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_C_COMPILER=gcc-13`.

The first configuration downloads and builds the dependencies, which takes a
few minutes and requires an Internet connection. Reuse the same `build/`
folder afterwards.

> [!WARNING]
> Don't use `cmake .` at the root: it configures in-source and pollutes
> the repo (`CMakeCache.txt`, `CMakeFiles/`, `Makefile`, ...).

---

## Install

Once the project is built, it can be installed system-wide, so the `game`
executable can be launched from anywhere without staying in the repo:

```sh
# linux
sudo cmake --install build

# windows (from an administrator terminal)
cmake --install build --config Release
```

This copies the `game` executable into `bin/` and the engine's libraries
(`engine-system`, `engine-graphical2D`, `engine-graphical3D`, `engine-network`,
`engine-audio`) into `lib/`, under the install prefix: `/usr/local` on Linux,
`C:\Program Files\R-Type` on Windows.

To install somewhere else, for example in your home folder without `sudo`,
pass a prefix:

```sh
cmake --install build --prefix ~/.local
```

---

## Usage

The assets are loaded relative to `Games/`, so run the executables from there:

```sh
cd Games
./r-type_server    # master server, listens on port 4242
./r-type_client    # in another terminal; enter the server's IP in the menu (127.0.0.1 by default)
```

Each game started from a lobby runs as its own instance, on a port between
4243 and 6767: open this range (UDP and TCP) on the server's firewall to play
over a network.

---

## Tests

```sh
cmake --build build --target engine_tests_run
cmake --build build --target client_tests_run
cmake --build build --target server_tests_run
```

- The engine, client and server tests use **Criterion** (the client and
  server ones with code coverage, `--coverage`). They are only built when
  Criterion is installed, and are currently disabled on Windows (see the
  [test framework study](docs/comparative-studies/en/11-test-framework.md)).
- The engine tests cover the ECS. To build them without the sanitizers,
  configure with `-DENGINE_TESTS_SANITIZE=OFF`.

---

## Documentation

Every document exists in French and English.

| Document | Content | |
|---|---|---|
| **Game making guide** | Create a game on top of the engine: folder, CMake, window, sprites, input, sound, do's and don'ts | [EN](docs/GAME_GUIDE.en.md) · [FR](docs/GAME_GUIDE.fr.md) |
| **Running a game guide** | Run a game with the `game` executable: `config.conf`, modules, folder, debug mode, do's and don'ts, errors and fixes | [EN](docs/ENGINE_GAME.en.md) · [FR](docs/ENGINE_GAME.fr.md) |
| **Lua scripting guide** | Write a game's content in Lua: entities, components, systems, views, the frame, errors; exposing a component or a system to Lua | [EN](docs/LUA_GUIDE.en.md) · [FR](docs/LUA_GUIDE.fr.md) |
| **Extending the engine** | Add a component, a system, a module, a `gm:` function or a `config.conf` setting, and make them usable from Lua | [EN](docs/ENGINE_EXTENDING.en.md) · [FR](docs/ENGINE_EXTENDING.fr.md) |
| **ECS guide** | Entities, components, systems, views, the frame, errors, pitfalls, recipes, API cheat sheet | [EN](docs/ECS_GUIDE.en.md) · [FR](docs/ECS_GUIDE.fr.md) |
| **ECS design notes** | Why the ECS is built the way it is | [EN](docs/ECS_DESIGN.en.md) · [FR](docs/ECS_DESIGN.fr.md) |
| **Accessibility** | What the engine offers for accessibility (rebinding, layouts, input, text, tint), why it helps and who for | [EN](docs/ACCESSIBILITY.en.md) · [FR](docs/ACCESSIBILITY.fr.md) |
| **Comparative studies** | Why Windows and Linux, C++, CMake, an ECS, raylib, Asio, `plf::colony`, sparse sets, `FetchContent`, UDP, Criterion | [EN](docs/comparative-studies/en/README.md) · [FR](docs/comparative-studies/fr/README.md) |

The engine's headers (`Engine/*/include`) also carry Doxygen comments for
every class and component.

---

## Development

```sh
./script/clang.sh         # check formatting (clang-format)
./script/clang.sh --fix   # fix formatting
./script/lint.sh          # check coding style (clang-tidy, uses build/compile_commands.json)
```

The CI (GitHub Actions, Ubuntu 24.04) builds the client and the server, runs
the Criterion tests and the linter on every push and pull request. The linter also
checks the code against C++26 (`-std=c++2c`).

To write a game's Lua scripts faster, a VS Code extension for the engine's Lua
is available: **R-Type lua engine epitech**, published by Ariel Amriou
(publisher ID `ariel-amriou`). Install it with:

```sh
code --install-extension ariel-amriou.r-type-lua-engine-epitech
```

or search for "R-Type lua engine epitech" in the VS Code Marketplace
(Extensions view, `Ctrl+Shift+X`).

---

## Contributing

Pick the guide that matches what you change:

| You want to... | Read |
|---|---|
| Understand the ECS before touching it | [ECS guide](docs/ECS_GUIDE.en.md) and [ECS design notes](docs/ECS_DESIGN.en.md) |
| Add a component, a system, a module, a `gm:` function or a `config.conf` setting | [Extending the engine](docs/ENGINE_EXTENDING.en.md) |
| Write or change a game on top of the engine | [Game making guide](docs/GAME_GUIDE.en.md), [running a game](docs/ENGINE_GAME.en.md), [Lua scripting guide](docs/LUA_GUIDE.en.md) |
| Change the network protocol | [Protocol specification](docs/ENGINE_PROTOCOL.txt), to keep in sync with the code |
| Know why a library or a tool was chosen | [Comparative studies](docs/comparative-studies/en/README.md) |

Every guide also exists in French (`*.fr.md`). When you change one, update
both languages.

Before opening a pull request against `dev`:

- build the project, then run `./script/clang.sh` and `./script/lint.sh`
  (see [Development](#development)): the CI fails if either reports a problem;
- run the [tests](#tests), especially `engine_tests_run` after any change to
  the ECS.

### Commits

Commits follow [Conventional Commits](https://www.conventionalcommits.org/):
`type(scope): summary`.

- **Type:** `feat`, `fix`, `docs`, `refactor` or `style`.
- **Scope:** the area touched, as a path (`engine`, `engine/game`,
  `engine/lua`). Leave it out for docs that cover the whole repo.
- **Summary:** in English, lowercase, imperative, no trailing period, and
  saying what changes for the user, e.g.
  `feat(engine/game): stop the game from Lua with gm:stop()`.
- **Body** (optional, when the change needs explaining): plain prose wrapped
  at about 72 columns, explaining the behavior and the why. Code identifiers
  are written as-is, without backticks.
- **One logical change per commit:** code and its docs go in separate commits
  (`feat(...)`, then `docs(...)`).

### Branches and pull requests

- **Branch:** `type/short-name` (`feat/cmake-install`, `docs/vscode-extension`),
  opened against `dev`.
- **PR title:** the branch name, capitalized (`Feat/cmake install`).
- **PR body:** one past-tense sentence summarizing the change ("Added an
  install step: ..."), then a short `-` bullet list of the concrete points with
  identifiers in backticks, then `Closes #N` when there is an issue. No
  headings, no test plan section.

---

## Clean

```sh
cmake --build build --target clean   # compiled objects only
rm -rf build                          # reconfigure everything from scratch
```

---

## Troubleshooting

| Problem | Fix |
|---|---|
| Compilation errors on C++20 features (`<format>`, concepts, ...) | The compiler is too old (`g++ --version` must say 13 or more). Install `g++-13` or newer and reconfigure in a fresh folder: `rm -rf build`, then `cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_C_COMPILER=gcc-13`. |
| CMake fails while configuring raylib (`X11`, `Xrandr`, `GL` not found) | The X11/OpenGL development packages are missing: install them (see [Requirements](#requirements)) and reconfigure. |
| The first configuration fails while downloading a dependency | It needs an Internet connection and `git`. Check both, delete `build/` and configure again. |
| CMake complains about a different compiler than the cache | The compiler was changed on an existing `build/`: delete it (`rm -rf build`) and configure again. |
| `make: *** No rule to make target 'engine_tests_run'` | Criterion wasn't found when configuring: install `libcriterion-dev`, then reconfigure. |
| `cannot find -lasan` / `-lubsan` when building the engine tests | The sanitizer runtimes are missing: install `libasan8 libubsan1`, or build without them with `-DENGINE_TESTS_SANITIZE=OFF`. |
| Engine tests stop at startup with `AddressSanitizer:DEADLYSIGNAL` | Some kernels use an address randomization ASan doesn't support: run `sudo sysctl vm.mmap_rnd_bits=28`, or build with `-DENGINE_TESTS_SANITIZE=OFF`. |
| The client opens but sprites or sounds are missing | It was launched from the wrong folder: `cd Games` first, then `./r-type_client`. |
| The client can't reach the server | Check that `r-type_server` is running, that the IP entered in the menu is right, and that ports 4242 to 6767 are open on the server. |
| The project was configured with `cmake .` by mistake | Delete the generated files (`CMakeCache.txt`, `CMakeFiles/`, `Makefile`, `cmake_install.cmake`) at the root and in the subfolders, then use `cmake -S . -B build`. |

---

## Authors

| Name | GitHub |
|---|---|
| Ariel Amriou | [@ArielAmriou](https://github.com/ArielAmriou) |
| Pierrick Simon | [@pierrick-simon](https://github.com/pierrick-simon) |
| Lucas Sauvage | [@Zerrias0](https://github.com/Zerrias0) |
| Aaron Joseph | [@tomaree](https://github.com/tomaree) |

---

## License

This project is distributed under the [MIT License](LICENSE).
