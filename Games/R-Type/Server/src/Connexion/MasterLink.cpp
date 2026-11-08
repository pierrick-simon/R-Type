/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterLink
*/

#include "Connexion/MasterLink.hpp"

#include <iostream>

#include "Connexion/Instance.hpp"
#include "Connexion/InstanceLink.hpp"
#include "UserDB.hpp"

namespace RType::Server {

    namespace {

        void acceptInstances(MasterLink &link)
        {
            while (true) {
                asio::ip::tcp::socket socket(*link.io);
                std::error_code ec;

                link.acceptor->accept(socket, ec);
                if (ec)
                    return;
                link.instances.emplace_back(TcpConnection(std::move(socket)));
            }
        }

        void readMessages(LinkedInstance &instance, UserDB &db)
        {
            while (auto message = instance.connection.nextMessage()) {
                const auto &p = message->payload;

                if (message->type == LINK_HELLO && p.size() == 2) {
                    instance.port = get16(p.data());
                    instance.since = std::chrono::steady_clock::now();
                }
                if (message->type == LINK_SCORE && !p.empty() &&
                    p.size() == 1u + p[0] + 4u) {
                    const std::string user(p.begin() + 1, p.begin() + 1 + p[0]);
                    const long score = get32(p.data() + 1 + p[0]);

                    if (db.recordScore(user, score))
                        std::cout << "[Master] New best score for " << user
                                  << ": " << score << std::endl;
                }
                if (message->type == LINK_STATUS && p.size() == 2) {
                    instance.players = p[0];
                    instance.started = p[1] != 0;
                    if (instance.players > 0)
                        instance.everHadPlayers = true;
                    std::cout << "[Master] Lobby " << instance.port << ": "
                              << instance.players << " player(s)"
                              << (instance.started ? ", in game" : "")
                              << std::endl;
                }
            }
        }

        bool shouldClose(const LinkedInstance &instance)
        {
            const auto age = std::chrono::steady_clock::now() - instance.since;

            if (instance.port == 0 || instance.stopSent || instance.players > 0)
                return false;
            return instance.everHadPlayers || age >= EMPTY_LOBBY_TIMEOUT;
        }

    } // namespace

    MasterLink::MasterLink() :
        io(std::make_unique<asio::io_context>()),
        acceptor(std::make_unique<asio::ip::tcp::acceptor>(*io,
            asio::ip::tcp::endpoint(
                asio::ip::address_v4::loopback(), MASTER_LINK_PORT)))
    {
        acceptor->non_blocking(true);
    }

    void systemMasterLink(GameManager &gm)
    {
        gm.view<MasterLink, InstanceManager, UserDB>().each(
            [](Entity, MasterLink &link, InstanceManager &threads, UserDB &db) {
                acceptInstances(link);
                for (auto &instance : link.instances) {
                    instance.connection.poll();
                    readMessages(instance, db);
                    if (shouldClose(instance)) {
                        std::cout << "[Master] Lobby " << instance.port
                                  << " is empty, closing it" << std::endl;
                        instance.connection.send(LINK_STOP);
                        instance.connection.poll();
                        instance.stopSent = true;
                    }
                }
                std::erase_if(link.instances, [&](LinkedInstance &instance) {
                    if (!instance.connection.isClosed())
                        return false;
                    if (instance.port != 0) {
                        reapInstance(threads, instance.port);
                        std::cout << "[Master] Lobby " << instance.port
                                  << " closed" << std::endl;
                    }
                    return true;
                });
            });
    }

} // namespace RType::Server
