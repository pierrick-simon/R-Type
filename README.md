# R-Type

![C++](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%E2%89%A54.4-064F8C?logo=cmake&logoColor=white)
![raylib](https://img.shields.io/badge/raylib-engine-black?logo=raylib&logoColor=white)
![Epitech](https://img.shields.io/badge/Epitech-2026-000000)

A multiplayer **R-Type** game written in C++26, Epitech project (2026). The
repository is organized around a shared engine (`Engine`) consumed by a
server and a client (`Games/R-Type`).

> [!NOTE]
> The project is still in its early stages: the CMake structure, the
> engine layout and the client/server skeletons are in place, but the
> game logic itself is yet to be implemented.

## Architecture

```
R-Type/
├── Engine/                 # Shared code, independent from the game
│   ├── Shared/include/     # Common headers (interface lib `shared`)
│   └── System/             # Engine (ECS, raylib rendering, etc.) -> `engine-system` lib
└── Games/
    └── R-Type/
        ├── Client/         # `r-type_client` executable
        └── Server/         # `r-type_server` executable
```

- **`Engine`** doesn't depend on any game code: it exposes reusable
  libraries (`shared`, `engine-system`) consumed by `Games`, and it's the
  one that depends on **raylib**.
- **`Games/R-Type`** contains the client and the server, both of which
  rely on the engine, each with its own sources, headers and tests.

## Requirements

- **CMake** ≥ 4.4
- **GCC** ≥ 14 (`g++-14`) — required for the **C++26** dialect
- **raylib** for the engine (`Engine`)
- **Criterion** for tests (automatically downloaded if missing)

## Installation

```sh
# linux
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14
cmake --build build -j

#windows
cmake -S . -B build
cmake --build build --config Release
```

> [!WARNING]
> Don't use `cmake .` at the root: it configures in-source and pollutes
> the repo (`CMakeCache.txt`, `CMakeFiles/`, `Makefile`, ...).

## Usage

```sh
./Games/r-type_server
./Games/r-type_client
```

## Tests

```sh
cmake --build build --target client_tests_run
cmake --build build --target server_tests_run
```

## Clean

```sh
cmake --build build --target clean   # compiled objects only
rm -rf build                          # reconfigure everything from scratch
```

## Linter

```sh
./script/clang.sh     # for format error
./script/lint.sh      # for codding style error
```
