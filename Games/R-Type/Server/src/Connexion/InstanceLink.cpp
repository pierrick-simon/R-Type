/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** InstanceLink
*/

#include "Connexion/InstanceLink.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

#include "Connexion/Lobby.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "LobbyProtocol.hpp"
#include "Player/Player.hpp"
#include "ServerManager.hpp"

namespace RType::Server {

    asio::ip::tcp::socket InstanceLink::connectToMaster(asio::io_context &io)
    {
        asio::ip::tcp::socket socket(io);

        socket.connect(asio::ip::tcp::endpoint(
            asio::ip::address_v4::loopback(), MASTER_LINK_PORT));
        return socket;
    }

    InstanceLink::InstanceLink(uint16_t instancePort) :
        io(std::make_unique<asio::io_context>()),
        connection(connectToMaster(*io))
    {
        std::vector<uint8_t> hello(2);

        put16(hello.data(), instancePort);
        connection.send(LINK_HELLO, hello);
    }

    void systemInstanceLink(GameManager &gm)
    {
        int players = 0;

        gm.view<ServerManager>().each([&players](Entity, ServerManager &s) {
            players = static_cast<int>(s.getClients().size());
        });
        gm.view<InstanceLink>().each([&](Entity, InstanceLink &link) {
            const bool started = isGameStarted();

            if (players != link.lastPlayers || started != link.lastStarted) {
                link.lastPlayers = players;
                link.lastStarted = started;
                link.connection.send(LINK_STATUS,
                    {static_cast<uint8_t>(players),
                        static_cast<uint8_t>(started ? 1 : 0)});
            }
            link.connection.poll();
            while (auto message = link.connection.nextMessage())
                if (message->type == LINK_STOP)
                    link.stopRequested = true;
            if (link.connection.isClosed() && !link.stopRequested) {
                std::cerr << "[Instance] Lost the master link, stopping"
                          << std::endl;
                link.stopRequested = true;
            }
        });
    }

    void reportScores(GameManager &gm)
    {
        for (auto [_, link] : gm.view<InstanceLink>().getVector()) {
            for (auto [entity, player, member, scorer] :
                gm.view<Player, LobbyMember, Scorer>().getVector()) {
                if (member.name.empty())
                    continue;
                const auto length = static_cast<uint8_t>(
                    std::min<std::size_t>(member.name.size(), 255));
                std::vector<uint8_t> data(1 + length + 4);

                data[0] = length;
                std::copy_n(member.name.begin(), length, data.begin() + 1);
                put32(data.data() + 1 + length,
                    static_cast<uint32_t>(std::max(scorer.score, 0)));
                link.connection.send(LINK_SCORE, data);
            }
            link.connection.poll();
        }
    }

} // namespace RType::Server
