# 01 · Why Windows and Linux?

> 🌐 **Language:** [Français](../fr/01-systemes-exploitation.md) · English

> **Decision:** the game (client and server) targets **Windows and Linux**.
> **Status:** adopted · **Date:** October 2026 · [Back to the index](README.md)

## In short

Windows is where the players are: about 94% of Steam players. Linux is where the servers run: more than 90% of known web servers run a Unix system. It is also the team's development and continuous integration system. Targeting both covers the **client** as well as the **server**. macOS, consoles and the web are ruled out: they are expensive to test for an audience gain that is small or out of reach.

---

## Context

A multiplayer R-Type is two programs:

- **the client** (`r-type_client`), which displays the game and reads the keyboard: it runs **on the player's machine**;
- **the server** (`r-type_server`), which is authoritative over the game: it runs **on a hosted machine**, without a screen.

The two do not have the same audience. The question is therefore twofold: which systems do players play on, and which systems are servers hosted on?

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Player audience** | 3 | A game that players cannot launch is useless |
| **Testing cost** | 2 | Every targeted system must be built and tested regularly, on a real machine |
| **Tool support** | 2 | Compilers, raylib, Asio, plf::colony and CMake must work there |
| **Server hosting** | 2 | The server must run where machines can be rented |

---

## The data

### Where the players are: the Steam survey

Valve publishes a monthly survey of the hardware and software of Steam users, the largest PC gaming platform. Results for **July 2026**, the most recent found ([GamingOnLinux](https://gamingonlinux.com/2026/08/linux-back-over-4-percent-in-the-july-2026-steam-survey), [Mezha](https://mezha.ua/en/news/steam-survey-linux-part-313817/), [Valve's official page](https://store.steampowered.com/hwsurvey)):

| System | Share of Steam players |
|---|---|
| Windows | **93.67%** |
| Linux | **4.01%** |
| macOS | **2.32%** |

- **Linux is growing:** it crossed 5% for the first time in March 2026, driven by SteamOS (Steam Deck), then fell back to around 4% ([KitGuru](https://www.kitguru.net/gaming/joao-silva/steamos-propels-linux-to-record-5-33-share-in-latest-steam-survey/)). It is now **ahead of macOS**.
- **Limitation:** the survey is voluntary, and its population changes from one month to the next. Monthly variations of one point should therefore be taken with caution. The order of magnitude, however, has been stable for years: Windows dominates by far.

### Where the servers are

According to W3Techs, on October 19, 2026, **92.2%** of websites whose system is known run a **Unix**-like system (overwhelmingly Linux), against **8.0%** for Windows ([W3Techs](https://w3techs.com/technologies/overview/operating_system)). This figure is about web servers, not game servers specifically, but it reflects what hosting providers offer: a Linux machine is the norm, cheaper (no license) and without a graphical interface.

### The case of macOS

Apple **deprecated OpenGL** with macOS 10.14 Mojave, in 2018, in favor of its proprietary Metal API. According to Apple's announcement, "*apps built using OpenGL and OpenCL will continue to run in macOS 10.14*", but these APIs are no longer developed ([Geeks3D](https://www.geeks3d.com/20180605/apple-announces-that-opengl-and-opencl-will-be-deprecated-in-macos-10-14-mojave/)). raylib relies on OpenGL: it works on macOS, but on a foundation Apple no longer evolves. Testing on macOS also requires an Apple machine.

---

## The options considered

### Windows + Linux (chosen)

**Strengths**

- **Covers almost the whole player audience** (Windows, plus Linux and Steam Deck) **and hosting** (Linux).
- **Development and continuous integration already happen on Linux**, and the code stays portable to Windows thanks to cross-platform libraries.
- **Every tool of the project is tested on both**: GCC/Clang on Linux, MSVC on Windows. Asio lists Linux (GCC, Clang) and Windows (Visual C++ 2013+) among its tested platforms ([Asio](https://think-async.com/Asio/)), and raylib lists Windows and Linux among its platforms ([raylib](https://github.com/raysan5/raylib)).

**Weaknesses**

- **Two systems to maintain**: two compilers, and API differences (sockets, shared libraries, file encoding) that must be abstracted away.

### Linux only

**Strengths**

- A single environment, the one used by continuous integration (`ubuntu-24.04` in `.github/workflows/`).

**Weaknesses**

- **Only about 4% of players.**

### Windows only

**Strengths**

- The largest player audience.

**Weaknesses**

- The server would have to run on Windows, a minority and more expensive setup at hosting providers.

### Windows + Linux + macOS

**Strengths**

- About 2% more players.

**Weaknesses**

- **A third toolchain** (Apple Clang, Xcode), an Apple machine to test on, and a deprecated OpenGL. The cost clearly outweighs the gain.

### Consoles, mobile, web

**Weaknesses**

- **Consoles:** development kits are reserved for studios under non-disclosure agreements.
- **Mobile:** a touch interface does not suit a keyboard shoot'em up, and porting requires other tooling (Android NDK, Xcode).
- **Web:** raylib can compile to HTML5, but a browser cannot open a raw UDP socket, which rules out the planned network protocol ([study 06](06-network-library.md)).

---

## Comparison grid

| Criterion (weight) | Windows + Linux | Linux only | Windows only | + macOS | Web |
|---|---|---|---|---|---|
| Player audience (3) | 5 | 1 | 5 | 5 | 3 |
| Testing cost (2) | 4 | 5 | 5 | 2 | 3 |
| Tool support (2) | 5 | 5 | 5 | 3 | 2 |
| Server hosting (2) | 5 | 5 | 2 | 5 | 1 |
| **Weighted total /100** | **96** | **73** | **87** | **78** | **47** |

---

## Decision and consequences

**The project targets Windows and Linux.** This is already visible in the repository:

- the root `CMakeLists.txt` enables MSVC-specific options (`/utf-8`, `/permissive-`, `/Zc:__cplusplus`);
- `CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS` lets the engine's shared libraries work on Windows, where the symbols of a DLL are not exported by default;
- Asio is configured for Windows 10 and later (`_WIN32_WINNT=0x0A00`) and linked to `ws2_32`, the Windows socket library;
- our `CMakeLists.txt` only enables Criterion, the test framework, on Unix, although it also works on Windows ([study 11](11-test-framework.md)).

**What we accept:**

- **Any system-specific code must go through an abstraction**, or through a library that provides one (raylib for rendering, Asio for networking).
- **Unit tests currently only run on Linux.** They will have to be enabled on Windows ([study 11](11-test-framework.md)), and a regular Windows build remains essential to catch MSVC-specific regressions.
- **Continuous integration currently only builds on Linux** (`ubuntu-24.04`). Adding a Windows build (`windows-latest`) would close that gap.

---

## When to revisit this choice

- **Linux stays above 10% on Steam**: the choice does not change, but Linux becomes a full player target, not just a server one.
- **The game must run in a browser** (online demo): a compatible transport (WebSocket or WebRTC) would be needed on top of UDP.
- **A team member develops on a Mac**: raylib and Asio work there, but the target would need to be documented and tested.

---

## Sources

- Valve, *Steam Hardware & Software Survey*: <https://store.steampowered.com/hwsurvey>
- GamingOnLinux, *Linux back over 4 percent in the July 2026 Steam Survey*: <https://gamingonlinux.com/2026/08/linux-back-over-4-percent-in-the-july-2026-steam-survey>
- Mezha, July 2026 Steam survey results: <https://mezha.ua/en/news/steam-survey-linux-part-313817/>
- KitGuru, *SteamOS propels Linux to record 5.33% share*: <https://www.kitguru.net/gaming/joao-silva/steamos-propels-linux-to-record-5-33-share-in-latest-steam-survey/>
- W3Techs, *Usage statistics of operating systems for websites*: <https://w3techs.com/technologies/overview/operating_system>
- Geeks3D, *Apple Announces that OpenGL and OpenCL will be Deprecated in macOS 10.14 Mojave*, June 2018: <https://www.geeks3d.com/20180605/apple-announces-that-opengl-and-opencl-will-be-deprecated-in-macos-10-14-mojave/>
- Asio, tested platforms: <https://think-async.com/Asio/>
- raylib, supported platforms: <https://github.com/raysan5/raylib>
