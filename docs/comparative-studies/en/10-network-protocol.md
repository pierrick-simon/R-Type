# 10 · UDP or TCP, with Asio?

> 🌐 **Language:** [Français](../fr/10-protocole-reseau.md) · English

> **Decision:** the game is played over **UDP**, with a connection and client tracking written on top. On the server, a single `asio::ip::udp::socket` serves every client.
> **Status:** adopted, implemented on the `feature/network` branch · **Date:** October 2026 · [Back to the index](README.md)

## In short

**TCP** delivers everything, in order. But when a packet is lost, all the following data waits for it to be resent: in an action game, the screen freezes then catches up. **UDP** delivers each datagram as soon as it arrives, or never, and a datagram is always a complete message. The project therefore chose **UDP**, with Asio. The implementation (`feature/network` branch) fits in a single socket for all players, a 9-byte header, an identifier-based connection and a regular presence check. Important messages are not resent on loss yet: that is the next step (see [Decision](#decision-and-consequences)).

---

## Context

`engine-network` relies on Asio ([study 06](06-network-library.md)), which offers both protocols: "*Asio provides off-the-shelf support for the internet protocols TCP, UDP and ICMP*" ([Asio, *TCP, UDP and ICMP*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html)). We need to choose which one carries the game.

What flows during a game:

| Message | Direction | Frequency | Can it be lost? |
|---|---|---|---|
| Player inputs (keys pressed) | client → server | Several dozen times per second | Yes: the next ones arrive right after |
| Game state (positions, health…) | server → clients | Several dozen times per second | Yes: the next one replaces it |
| Connection, start and end of game, death of a player | both | Rare | **No** |

Most of the traffic is therefore data that **expires very quickly**. Only a small part must arrive for sure.

---

## What distinguishes TCP and UDP

| | **TCP** ([RFC 9293](https://www.rfc-editor.org/rfc/rfc9293.html)) | **UDP** ([RFC 768](https://www.rfc-editor.org/rfc/rfc768.html)) |
|---|---|---|
| **Delivery** | Guaranteed, in order | Neither guaranteed nor ordered |
| **Unit of data** | A byte stream: "*there are no message boundaries*" ([Asio, *Streams*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html)) | A datagram, always received whole or not at all |
| **On loss** | The following data waits for the lost data to be resent | The following datagrams arrive normally |
| **Connection** | Explicitly established, disconnection detected | None: must be handled by hand |
| **Header size** | 20 bytes minimum | 8 bytes |
| **Rate control** | Built in (congestion control) | Must be done by hand: an application "*SHOULD control the rate*" of its sends ([RFC 8085, section 3.1](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.1)) |
| **Small sends** | May be delayed to be grouped together (Nagle's algorithm, [RFC 896](https://www.rfc-editor.org/rfc/rfc896.html)) | Leave immediately |

The deciding point is the behavior on loss. Glenn Fiedler, a reference on game networking, describes it this way for TCP: new data "*gets put in a queue, and you cannot access it until that lost packet has been retransmitted*". He concludes: "*Using TCP is the worst possible mistake you can make when developing a multiplayer game!*" ([Gaffer On Games, *UDP vs. TCP*](https://gafferongames.com/post/udp_vs_tcp/)).

---

## What it changes in the Asio code

### With TCP

- **An `asio::ip::tcp::acceptor`** waits for connections, and **each client has its own `asio::ip::tcp::socket`**.
- **The stream must be split by hand**, since it has no boundaries: usually, each message is prefixed with its size, and exactly that number of bytes is read with `asio::async_read`. Asio's documentation warns that reads and writes may be partial ("*short read or short write*") ([Asio, *Streams*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html)).
- **Nagle's algorithm must be disabled** on every socket with `asio::ip::tcp::no_delay`, so that small messages leave right away ([Asio, *ip::tcp::no_delay*](https://think-async.com/Asio/asio-1.38.2/doc/asio/reference/ip__tcp/no_delay.html)).
- **In return**, connection, disconnection, ordering and reliability are provided by the system.

### With UDP

- **A single `asio::ip::udp::socket`**, bound to the server's port, receives the datagrams of every client.
- **`async_receive_from`** gives, for each datagram received, the address of its sender (its *endpoint*). The server keeps an "endpoint → player" table ([Asio, *TCP, UDP and ICMP*](https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html)).
- **A datagram is a complete message**: no splitting to do.
- **In return**, there is no connection, no ordering and no reliability: the game must build them, where it needs them.

The core of a UDP server with Asio, modeled on `UDPServer::startReceive` in the project:

```cpp
class UdpServer {
public:
    UdpServer(asio::io_context &io, std::uint16_t port) :
        _socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    {
        receive();
    }

private:
    asio::ip::udp::socket _socket;
    asio::ip::udp::endpoint _sender;
    std::array<std::uint8_t, 1200> _buffer {};

    void receive()
    {
        _socket.async_receive_from(asio::buffer(_buffer), _sender,
            [this](std::error_code ec, std::size_t size) {
                if (!ec)
                    handlePacket(_sender, size);   // one datagram = one complete message
                receive();
            });
    }

    void handlePacket(const asio::ip::udp::endpoint &from, std::size_t size);
};
```

---

## Criteria

| Criterion | Weight | Why it matters |
|---|---|---|
| **Latency on loss** | 3 | A loss must not freeze the game |
| **Reliability and ordering provided** | 2 | Important messages must arrive |
| **Connection management** | 2 | Know who is connected, and detect departures |
| **Simplicity of the Asio code** | 2 | Less network code means fewer bugs that are hard to reproduce |
| **Overhead per message** | 1 | Headers, grouping of small sends |
| **Server load** | 1 | Number of sockets and pending reads |

---

## The options considered

### TCP only

**Strengths**

- Reliability, ordering, connection and rate control provided by the system.
- No in-house layer to write.

**Weaknesses**

- **A loss blocks the whole stream** until it is resent: exactly what an action game must avoid.
- Message splitting to write (size prefix), Nagle to disable, one socket per client.

### UDP only, with an in-house layer (chosen)

**Strengths**

- **A loss delays nothing:** the next datagram arrives normally.
- **A datagram is a message**, and a single socket serves every client.
- **Only what must be reliable is made reliable** (connection, events); the rest leaves without waiting.

**Weaknesses**

- **Connection, disconnection, acknowledgements and rate control must be written.**
- Datagram size must be limited by hand (see [Decision](#decision-and-consequences)).

### TCP and UDP together (TCP for important messages, UDP for play)

**Strengths**

- Free reliability for connection and events, UDP responsiveness for play.

**Weaknesses**

- **Two protocols, two code paths, two sockets per client**, to keep in sync.
- **They interfere:** according to Glenn Fiedler, "*TCP tends to induce packet loss in UDP packets*" ([Gaffer On Games](https://gafferongames.com/post/udp_vs_tcp/)). The ENet authors, who studied this combination for their shooter *Cube*, conclude that the result is "*worse off in features and performance*" than a single protocol on top of UDP ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).

---

## The data

- **ENet** was born from a multiplayer shooter, *Cube*, which sent data very frequently with low latency: "*TCP was an unsuitable choice due to its high latency and stream orientation*" ([ENet, *Features and Architecture*](https://github.com/lsalzman/enet/blob/master/docs/design.dox)).
- **Quake 3** relies essentially on UDP: the server sends each client the difference between the current state and the last acknowledged state. Lost information is thus naturally resent in the next message, without a reliable channel for the game state ([Fabien Sanglard, *Quake 3 Source Code Review: Network Model*](https://fabiensanglard.net/quake3/network.php)).
- **The IETF guidelines for UDP** ask applications to control the rate of their sends, and not to send datagrams larger than what the network path accepts ([RFC 8085](https://www.rfc-editor.org/rfc/rfc8085.html)).

---

## Comparison grid

| Criterion (weight) | TCP only | UDP + in-house layer | TCP + UDP |
|---|---|---|---|
| Latency on loss (3) | 1 | 5 | 4 |
| Reliability and ordering provided (2) | 5 | 2 | 4 |
| Connection management (2) | 5 | 2 | 4 |
| Simplicity of the Asio code (2) | 3 | 4 | 2 |
| Overhead per message (1) | 3 | 5 | 4 |
| Server load (1) | 3 | 5 | 3 |
| **Weighted total /100** | **64** | **75** | **71** |

---

## Decision and consequences

**The game is played over UDP.** This is what `UDPServer` and `UDPClient` implement in `Engine/Network`, on the `feature/network` branch:

| Element | Current implementation |
|---|---|
| **Sockets** | A single `asio::ip::udp::socket` on the server for every client, one per client |
| **Header (9 bytes)** | `type` (`uint8`, the message code), `payloadSize` (`uint16`), `sequence` (`uint32`), `senderId` (`uint16`), written byte by byte in big-endian (`put16`, `put32`) |
| **Validation** | A datagram shorter than the header, or whose size does not match `payloadSize`, is ignored. A message from an unknown identifier, or from another address than the one registered for that identifier, is ignored |
| **Connection** | The client sends `LOGIN` every 3 seconds until it receives its identifier, assigned by the server. `LOGOUT` allows an explicit departure |
| **Presence** | The server sends `HEALTH` every 2 seconds, and the client answers. A client silent for 6 seconds is removed |
| **Client messages** | Inputs: `KEY_PRESSED`, `KEY_RELEASED`, `MOUSE_BUTTON_PRESSED`, `MOUSE_BUTTON_RELEASED` |
| **Server messages** | Entity changes: `CREATE`, `MOVE`, `SET`, `TEXT`, `DESTROY`, and sounds: `PLAY`, `PAUSE`, `STOP` |
| **Size** | 1,024-byte receive buffer: every datagram stays below the recommended 1,200 bytes |
| **Game loop** | No network thread: an ECS system (`ServerManager::systemServerReceived`, `ClientManager::systemClientReceived`) calls `poll()` on the Asio context every frame |

These choices follow the recommendations of this study's sources: the server receives inputs, never positions, and does not take the client at its word ([Gambetta, *Client-Server Game Architecture*](https://www.gabrielgambetta.com/client-server-game-architecture.html)); datagrams stay under 1,200 bytes, the conservative limit used by QUIC ([RFC 9000, section 14](https://www.rfc-editor.org/rfc/rfc9000.html#section-14)) and by Glenn Fiedler ([Gaffer On Games, *Packet Fragmentation and Reassembly*](https://gafferongames.com/post/packet_fragmentation_and_reassembly/)).

**What we accept:** a connection layer to write and test, which TCP would have provided.

### Next steps

The current implementation has no reliability yet: a lost message is never resent. This does not matter for `MOVE` (the next position replaces it) or for inputs, but it does for some messages:

1. **Make `CREATE` and `DESTROY` reliable.** A lost `CREATE` leaves the client without the entity, and the following `MOVE` messages target it for nothing. A lost `DESTROY` leaves a ghost entity on screen. These messages must be resent until acknowledged.
2. **Add acknowledgements.** The `sequence` number already exists. What remains is to send back, in every packet, the last number received and a 32-bit field saying which of the 32 previous packets arrived ([Gaffer On Games, *Reliability and Congestion Avoidance over UDP*](https://gafferongames.com/post/reliability_ordering_and_congestion_avoidance_over_udp/)).
3. **Identify the protocol.** A game and version identifier at the start of each packet would let foreign datagrams, or datagrams from an old version, be ignored right away.
4. **Limit the rate.** The server must cap the number of datagrams accepted per client and per second, and control its own sending pace ([RFC 8085, section 3.1](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.1)).

---

## When to revisit this choice

- **The game adds large, non-urgent out-of-game exchanges** (chat lobby, level downloads): TCP becomes relevant for those flows, keeping in mind that it may disturb UDP traffic.
- **The reliability layer becomes too heavy to maintain**: switch to a library that provides it on top of UDP, such as ENet or GameNetworkingSockets ([study 06](06-network-library.md)).
- **The game must run in a browser**: a browser cannot open a raw UDP socket, WebSocket or WebRTC would be needed.

---

## Sources

- IETF, RFC 768, *User Datagram Protocol*: <https://www.rfc-editor.org/rfc/rfc768.html>
- IETF, RFC 9293, *Transmission Control Protocol (TCP)*: <https://www.rfc-editor.org/rfc/rfc9293.html>
- IETF, RFC 896, *Congestion Control in IP/TCP Internetworks* (John Nagle): <https://www.rfc-editor.org/rfc/rfc896.html>
- IETF, RFC 8085, *UDP Usage Guidelines*: <https://www.rfc-editor.org/rfc/rfc8085.html>
- IETF, RFC 8200 (IPv6), minimum link MTU: <https://www.rfc-editor.org/rfc/rfc8200.html>
- IETF, RFC 9000 (QUIC), section 14 *Datagram Size*: <https://www.rfc-editor.org/rfc/rfc9000.html#section-14>
- Asio, *TCP, UDP and ICMP*: <https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/networking/protocols.html>
- Asio, *Streams, Short Reads and Short Writes*: <https://think-async.com/Asio/asio-1.38.2/doc/asio/overview/core/streams.html>
- Asio, *ip::tcp::no_delay*: <https://think-async.com/Asio/asio-1.38.2/doc/asio/reference/ip__tcp/no_delay.html>
- Glenn Fiedler, *UDP vs. TCP*: <https://gafferongames.com/post/udp_vs_tcp/>
- Glenn Fiedler, *Reliability and Congestion Avoidance over UDP*: <https://gafferongames.com/post/reliability_ordering_and_congestion_avoidance_over_udp/>
- Glenn Fiedler, *Packet Fragmentation and Reassembly*: <https://gafferongames.com/post/packet_fragmentation_and_reassembly/>
- ENet, *Features and Architecture*: <https://github.com/lsalzman/enet/blob/master/docs/design.dox>
- Fabien Sanglard, *Quake 3 Source Code Review: Network Model*: <https://fabiensanglard.net/quake3/network.php>
- Gabriel Gambetta, *Client-Server Game Architecture*: <https://www.gabrielgambetta.com/client-server-game-architecture.html>
