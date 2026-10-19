# 06 · Why Asio for networking?

> 🌐 **Language:** [Français](../fr/06-bibliotheque-reseau.md) · English

> **Decision:** the engine's network library (`engine-network`) will rely on **Asio, in its standalone version** (without Boost).
> **Status:** adopted, implemented on the `feature/network` branch · **Date:** October 2026 · [Back to the index](README.md)

## In short

Asio offers **a single UDP and TCP socket API for Windows and Linux**, with an **asynchronous model** that lets a server handle several clients without one thread per client. Its standalone version **only needs standard C++11**, no Boost. It is the most proven C++ networking library: it even served as the basis for the C++ standard networking project (the *Networking TS*). The game-oriented alternatives (ENet, GameNetworkingSockets) impose their own protocol, whereas the project wants to design its own, tailored to its ECS.

---

## Context

The R-Type is multiplayer: a **server** is authoritative over the game, and several **clients** send it their inputs and receive the game state, over the Internet.

On Linux, sockets are BSD sockets (`<sys/socket.h>`); on Windows, they are Winsock (`<winsock2.h>`), with different initialization, types and error codes. The mechanisms for waiting on several sockets also differ: `epoll` on Linux, *I/O Completion Ports* on Windows.

**Current state:** Asio is already integrated into the build (`Engine/CMakeLists.txt`, version `asio-1-34-2`, with `ASIO_STANDALONE` and `ASIO_NO_DEPRECATED`) and linked to `engine-network`. On `dev`, that library still only contains an empty file; the implementation (`UDPServer`, `UDPClient`, `ServerManager`, `ClientManager` and the protocol messages) lives on the `feature/network` branch.

---

## UDP or TCP?

The choice of transport is the topic of [study 10](10-network-protocol.md): the game will be played over **UDP**, because with TCP a single lost packet blocks all the following data until it is resent. TCP may remain useful later for out-of-game exchanges (lobby, chat). The library must therefore offer **both**.

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Windows and Linux** | 3 | [Study 01](01-operating-systems.md): the same network code must run on both |
| **Asynchronous model** | 3 | The server must handle several clients without blocking its game loop |
| **Protocol freedom** | 3 | The protocol must match the game state (ECS components) to control packet size and frequency |
| **UDP and TCP** | 2 | UDP for play, possibly TCP for the lobby |
| **Light dependencies** | 2 | Fetchable with `FetchContent`, without installation ([study 03](03-build-system.md)) |
| **Maturity, documentation** | 2 | Networking is the hardest part to debug |

---

## The options considered

### Standalone Asio (chosen)

**Strengths**

- **A single API for Windows and Linux.** Asio is tested on Linux (GCC 4.8+, Clang 3.0+), macOS, Windows (Visual C++ 2013+) and FreeBSD ([Asio](https://think-async.com/Asio/)). Internally, it uses the best mechanism of each system (`epoll`, IOCP, `kqueue`).
- **Asynchronous:** `async_receive_from` and `async_send_to` take a function called when the operation completes. A single thread, or a small pool of threads, serves every client.
- **UDP, TCP, timers and name resolution**: everything a game server needs.
- **No Boost:** according to its documentation, with C++11 or later, "*most of Asio may be used without any extra dependencies*" ([Asio](https://think-async.com/Asio/)). It is header-only: nothing to build separately.
- **The most proven:** created in 2003, accepted into Boost in 2005, according to Wikipedia ([*Asio*](https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library))). The *Networking TS* (ISO/IEC TS 19216:2018), the networking extension of C++, has a design "*based on the Asio library*" ([cppreference, *Networking TS*](https://en.cppreference.com/w/cpp/experimental/networking)).
- **Low level:** it carries bytes, without imposing a packet format. The protocol remains entirely ours to design, as close as possible to the game's needs.

**Weaknesses**

- **Learning curve:** the asynchronous model (callbacks, buffer lifetimes, `io_context`) is demanding.
- **Many headers:** including `asio.hpp` slows down the compilation of the files concerned.
- **Reliability must be built by hand** on top of UDP (sequence numbers, acknowledgements) where needed.

### Boost.Asio

**Strengths**

- The same code (Boost.Asio 1.38.2 ships with Boost 1.92.0), with the Boost ecosystem (Beast for HTTP, for instance).

**Weaknesses**

- **Depends on Boost**, a huge library, long to download and build, for no gain here.

### Native sockets (BSD + Winsock), wrapped by hand

**Strengths**

- **No dependency**, and full control.

**Weaknesses**

- **Two implementations to write and test** (BSD and Winsock), plus a waiting mechanism per system (`epoll`, IOCP) to avoid blocking. It amounts to rewriting part of Asio, with fewer tests.

### ENet

**Strengths**

- **Designed for games:** a C library that adds optional reliable, in-order delivery on top of UDP, along with channels and fragmentation of large packets ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).

**Weaknesses**

- **Imposes its own protocol** (headers, acknowledgements, channels), which adds to the game's own and fixes some of its choices.
- **UDP only**, and a service loop to poll, no asynchronous model.

### GameNetworkingSockets (Valve)

**Strengths**

- **Very complete:** reliable and unreliable messages over UDP, per-packet AES-GCM-256 encryption, NAT traversal for peer-to-peer, latency and loss simulation tools ([GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets)).

**Weaknesses**

- **Heavy dependencies:** a cryptography library (OpenSSL 1.1.1+ or libsodium), Google Protobuf whose compiler and library must be "*the same version*", and Abseil for recent Protobuf versions ([BUILDING.md](https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md)).
- **Imposes its protocol**, like ENet.

### SFML Network

**Strengths**

- A simple C++ API for UDP and TCP.

**Weaknesses**

- **SFML would have to be shipped** while rendering uses raylib ([study 05](05-graphics-library.md)).
- Blocking or non-blocking sockets with a selector, no real asynchronous model.

### Standard C++ networking?

**It does not exist.** The *Networking TS*, based on Asio, was not merged into C++20, despite a proposal to that effect by Asio's author ([P1446R0](https://wg21.link/p1446)). C++26 adopts `std::execution`, an asynchronous programming framework ([Wikipedia, *Asio*](https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library))), but the list of standard library headers still contains **no networking header** ([cppreference, *Standard library headers*](https://en.cppreference.com/w/cpp/header)). Today, a third-party library is still required.

---

## The data

| Library | GitHub stars | License | Transport | Imposed protocol | Dependencies |
|---|---|---|---|---|---|
| **Asio** | 6,220 | Boost Software License | UDP, TCP | No | None (C++11) |
| GameNetworkingSockets | 9,989 | BSD-3-Clause | UDP (messages) | Yes | OpenSSL or libsodium, Protobuf |
| ENet | 3,291 | MIT | UDP | Yes | None |

*Source: GitHub API, accessed October 19, 2026.*

---

## Comparison grid

| Criterion (weight) | Standalone Asio | Boost.Asio | Native sockets | ENet | GameNetworkingSockets | SFML Network |
|---|---|---|---|---|---|---|
| Windows and Linux (3) | 5 | 5 | 2 | 5 | 4 | 5 |
| Asynchronous model (3) | 5 | 5 | 2 | 2 | 3 | 2 |
| Protocol freedom (3) | 5 | 5 | 5 | 3 | 2 | 5 |
| UDP and TCP (2) | 5 | 5 | 5 | 3 | 3 | 5 |
| Light dependencies (2) | 4 | 2 | 5 | 5 | 1 | 2 |
| Maturity, documentation (2) | 5 | 5 | 3 | 4 | 5 | 4 |
| **Weighted total /100** | **97** | **92** | **71** | **72** | **60** | **77** |

---

## Decision and consequences

**`engine-network` will rely on standalone Asio**, with the definitions already in place (`ASIO_STANDALONE`, `ASIO_NO_DEPRECATED`, Windows 10 minimum, `ws2_32` and `mswsock` on Windows).

**Asio is wrapped in `engine-network`.** On the `feature/network` branch, `UDPServer` and `UDPClient` use Asio, and the game goes through `ServerManager` and `ClientManager`, two components whose systems process the network every frame. This separation aims at three benefits:

1. **The game depends on no network API:** it knows neither Asio, nor Winsock, nor BSD sockets.
2. **Asio does not meet raylib** in the same file: on Windows, Asio includes the Win32 headers, some of whose names clash with raylib ([study 05](05-graphics-library.md#decision-and-consequences)).
3. **The library can be replaced** by rewriting only `engine-network`.

> [!WARNING]
> This is not quite the case yet: `Network.hpp` includes `<asio.hpp>`, and `KeyboardProtocol.hpp` includes both `Network.hpp` and `KeyboardManager.hpp` (hence raylib). Asio and raylib therefore end up in the same files, including the client's `Main.cpp`. It must be checked that this compiles on Windows, or `<asio.hpp>` must be moved out of the public headers (only included in the network `.cpp` files).

**The protocol** (UDP, 9-byte header, identifier-based connection, presence check) is described in [study 10](10-network-protocol.md).

**What we accept:** learning the asynchronous model, and building reliability on top of UDP by hand where needed.

---

## When to revisit this choice

- **Asio becomes a burden** (compile times, an asynchronous model too complex for the need): write the abstraction on top of BSD sockets and Winsock (the "native sockets" option), keeping the same interface.
- **The game needs encryption, NAT traversal or peer-to-peer**: GameNetworkingSockets becomes interesting despite its dependencies.
- **C++ adopts real standard networking** (beyond `std::execution`): migrate to it.

---

## Sources

- Asio, official page: <https://think-async.com/Asio/>
- Wikipedia, *Asio (C++ library)*: <https://en.wikipedia.org/wiki/Asio_(C%2B%2B_library)>
- cppreference, *Networking TS*: <https://en.cppreference.com/w/cpp/experimental/networking>
- cppreference, *Standard library headers*: <https://en.cppreference.com/w/cpp/header>
- ENet, *Features and Architecture*: <https://github.com/lsalzman/enet/blob/master/docs/design.dox>
- Christopher Kohlhoff, P1446R0, proposal to merge the Networking TS into C++20: <https://wg21.link/p1446>
- ENet: <https://github.com/lsalzman/enet>
- Valve, GameNetworkingSockets: <https://github.com/ValveSoftware/GameNetworkingSockets>
- Valve, GameNetworkingSockets, *BUILDING.md*: <https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md>
- GitHub API (stars and licenses), accessed October 19, 2026
