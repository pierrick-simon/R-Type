# 05 · Why raylib?

> 🌐 **Language:** [Français](../fr/05-bibliotheque-graphique.md) · English

> **Decision:** the engine's rendering, audio and input rely on **raylib 5.5**, used through its C++ wrapper **raylib-cpp**.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

raylib is the only library compared here that offers **ready-to-use 2D and 3D** (cameras, models, shaders), with **audio and input included**, **no external dependency**, on Windows and Linux, under the zlib license. SFML is nicer to use in C++, but **explicitly refuses 3D**. SDL3 and GLFW are lower level: the whole renderer would have to be written by hand. The price of raylib: a C API with global state (a single window), rendering limited to OpenGL, and name clashes with `windows.h`.

---

## Context

The engine (`Engine/`) must display images, animations and text, play sounds and read the keyboard and mouse, on Windows and Linux. The R-Type is a 2D game, but the engine is designed to host **other games** (the `Games/` folder is meant to hold several) and to allow effects that 2D alone cannot provide:

- **3D backgrounds** seen from the side, with real depth and parallax (the "2.5D" style of recent R-Type games);
- **3D-modeled bosses** in a 2D game;
- **shaders** for effects (hit flash, distortion, post-processing);
- **cameras** (zoom, shake, follow).

A **full engine** (Unity, Unreal, Godot) is ruled out: it imposes its game loop, its object model and its editor, whereas the project builds its own reusable engine, with its own ECS and a server that runs without a display ([shared constraints](README.md#constraints-shared-by-all-the-studies)). What is needed is a **library**, not an engine.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Built-in 2D and 3D** | 3 | Backgrounds, bosses, shaders, cameras; reuse for other games |
| **API simplicity** | 3 | The whole team must be able to display a sprite quickly |
| **Windows and Linux** | 2 | [Study 01](01-operating-systems.md) |
| **Audio and input included** | 2 | One library rather than three to integrate |
| **Dependencies and CMake integration** | 2 | Fetchable with `FetchContent` without a system installation ([study 03](03-build-system.md)) |
| **Community and documentation** | 2 | Examples, answers to questions, active maintenance |
| **License** | 1 | Compatible with a distributed project |

---

## The options considered

### raylib 5.5 + raylib-cpp (chosen)

**Strengths**

- **2D and 3D in the same API.** raylib advertises "*full 3D support, including 3D shapes, models, billboards, heightmaps and more*", plus model and post-processing shaders ([raylib](https://github.com/raysan5/raylib)). Its API includes `Camera3D`, `BeginMode3D`, `LoadModel`, `DrawModel` and `LoadShader` ([`raylib.h`](https://github.com/raysan5/raylib/blob/master/src/raylib.h)), and models load from OBJ, IQM, glTF, VOX and M3D ([`config.h`](https://github.com/raysan5/raylib/blob/master/src/config.h)).
- **Everything is included:** window, rendering, text, audio (WAV, OGG, MP3, FLAC…), keyboard, mouse, gamepad. raylib claims "*no external dependencies, all required libraries are included*". This is what makes it buildable with `FetchContent`, without installing anything.
- **Very simple:** a window and a sprite fit in a few lines. The official website offers examples with their code, sorted by module (core, shapes, textures, text, 3D models, shaders, audio) and by difficulty level ([raylib examples](https://www.raylib.com/examples.html)).
- **The most popular** of the compared libraries on GitHub (see [Data](#the-data)), and actively maintained.
- **raylib-cpp** adds a C++ layer (classes, RAII, exceptions): the engine's components (`Texture2D`, `Font`, `Sound`) inherit directly from its classes.

**Weaknesses**

- **C API with global state:** a single window, and global functions (`DrawTexture`, `IsKeyDown`). This is why the engine's `Window` component refuses to be created twice.
- **OpenGL rendering only** (1.1 to 4.3, ES 2.0/3.0): no Vulkan, DirectX 12 or Metal. OpenGL is more than enough for an R-Type, but it is deprecated on macOS ([study 01](01-operating-systems.md#the-case-of-macos)).
- **Name clashes with `windows.h`:** raylib defines `CloseWindow`, `ShowCursor` or `Rectangle`, which also exist in the Win32 API. Including both in the same file does not compile without precautions ([raylib discussion #1569](https://github.com/raysan5/raylib/discussions/1569)).
- **raylib-cpp has a small community** (fewer than 1,000 stars): for a problem, one often has to go back to raylib's C documentation.

### SFML 3

**Strengths**

- **Object-oriented C++ API**, very pleasant (`sf::Sprite`, `sf::Texture`), with a network module and an audio module.

**Weaknesses**

- **No 3D, by design.** The official FAQ answers "*No*" to the 3D question: the team wants to keep SFML a 2D library. For 3D, one must create the context with SFML then write "*raw OpenGL*" ([SFML FAQ](https://www.sfml-dev.org/faq/general/)).
- **Requires a C++17 compiler** for SFML 3 ([SFML FAQ](https://www.sfml-dev.org/faq/general/)): not an issue here, since the project uses C++20.

### SDL3

**Strengths**

- **Very widespread** for windowing, input and audio.
- **A modern GPU API** (SDL_GPU), on top of Vulkan, Direct3D 12 and Metal ([SDL3 Wiki](https://wiki.libsdl.org/SDL3/CategoryGPU)).

**Weaknesses**

- **Low level:** for 3D, the whole pipeline must be written (buffers, shaders, matrices). On top of that, "*each backend (Vulkan, Metal, D3D12) requires a different shader format*" ([SDL3 Wiki](https://wiki.libsdl.org/SDL3/CategoryGPU)). It means writing a renderer before displaying the first model.

### GLFW + OpenGL

**Strengths**

- Full control over rendering, and GLFW is light and reliable.

**Weaknesses**

- **GLFW only handles the window and input.** An OpenGL loader, an image library, an audio library and a text renderer must be added, then all the rendering written. raylib itself uses GLFW internally on desktop ([`rcore_desktop_glfw.c`](https://github.com/raysan5/raylib/blob/master/src/platforms/rcore_desktop_glfw.c)).

---

## The data

### Popularity and licenses (GitHub, October 19, 2026)

| Library | GitHub stars | License | Language | Built-in 3D |
|---|---|---|---|---|
| **raylib** | **35,321** | zlib | C99 | **Yes** (models, cameras, shaders) |
| SDL | 16,834 | zlib | C | No (low-level GPU API) |
| GLFW | 15,407 | zlib | C | No (window only) |
| SFML | 12,074 | zlib | C++17 | **No, by design** |
| raylib-cpp | 970 | zlib | C++ | (raylib wrapper) |

*Source: GitHub API, accessed October 19, 2026. The number of stars measures popularity, not quality.*

All these libraries use the **zlib license**, which allows commercial use and static linking with closed-source code: the license does not separate the options.

### What raylib covers (according to its README)

| Area | Content |
|---|---|
| Platforms | Windows, Linux, macOS, Raspberry Pi, Android, HTML5… |
| Rendering | OpenGL 1.1, 2.1, 3.3, 4.3, ES 2.0, ES 3.0, and a software renderer |
| 3D | Shapes, models, billboards, heightmaps, model and post-processing shaders |
| Audio | Loading and streaming playback: WAV, QOA, OGG, MP3, FLAC, XM, MOD |
| Dependencies | None: everything is included |
| Language | C99, with bindings for more than 70 languages |

Source: [raylib README](https://github.com/raysan5/raylib).

---

## Comparison grid

| Criterion (weight) | raylib | SFML 3 | SDL3 | GLFW + OpenGL |
|---|---|---|---|---|
| Built-in 2D and 3D (3) | 5 | 1 | 3 | 3 |
| API simplicity (3) | 4 | 5 | 2 | 1 |
| Windows and Linux (2) | 5 | 5 | 5 | 5 |
| Audio and input included (2) | 5 | 5 | 4 | 1 |
| Dependencies, CMake (2) | 5 | 4 | 4 | 3 |
| Community, documentation (2) | 4 | 4 | 5 | 4 |
| License (1) | 5 | 5 | 5 | 5 |
| **Weighted total /100** | **93** | **79** | **75** | **57** |

---

## Decision and consequences

**The engine uses raylib 5.5 and raylib-cpp v5.5.0**, fetched with `FetchContent` in `Engine/CMakeLists.txt`. raylib is built as a shared library.

**raylib is confined to `engine-graphical`.** Game code manipulates engine components (`Sprite2D`, `Text2D`, `Sound`…), not raylib directly. The library could therefore be replaced by rewriting only `Engine/Graphical`. The server displays nothing. On the `feature/network` branch, it still links `engine-graphical`, because the protocol reuses its types (keyboard keys, sprites, sounds).

**What we accept:**

- **A single window**: a raylib limitation, carried over by the `Window` component.
- **raylib and `windows.h` must never meet in the same `.cpp` file.** On Windows, Asio includes the Win32 headers: the graphics and network libraries must therefore stay separate, which the split into `engine-graphical` and `engine-network` must guarantee. This is not yet the case on the `feature/network` branch ([study 06](06-network-library.md#decision-and-consequences)).
- **No Vulkan, DirectX or Metal**: no consequence for an R-Type.

---

## When to revisit this choice

- **The game needs modern rendering** (thousands of lights, deferred rendering, Vulkan): SDL3 and its GPU API would become relevant, at the cost of writing a renderer.
- **The project drops any 3D ambition** and the team prefers an object API: SFML 3 would be more comfortable.
- **Moving to raylib 6**, already available (6.2 was released on October 15, 2026; vcpkg and Conan offer 6.0, and raylib-cpp follows 6.0 with its 6.0.3 release): update raylib and raylib-cpp together, since their versions are tied, and check the API changes before migrating.

---

## Sources

- raylib, README: <https://github.com/raysan5/raylib>
- raylib, cheatsheet: <https://www.raylib.com/cheatsheet/cheatsheet.html>
- raylib, examples: <https://www.raylib.com/examples.html>
- raylib, discussion #1569 on clashes with `windows.h`: <https://github.com/raysan5/raylib/discussions/1569>
- raylib-cpp: <https://github.com/RobLoach/raylib-cpp>
- SFML, general FAQ: <https://www.sfml-dev.org/faq/general/>
- SDL3, *CategoryGPU*: <https://wiki.libsdl.org/SDL3/CategoryGPU>
- Star History, graphics library comparison: <https://www.star-history.com/compare/graphics-library>
- GitHub API (stars and licenses), accessed October 19, 2026
