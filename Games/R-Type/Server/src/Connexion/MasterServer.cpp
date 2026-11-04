/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterServer
*/

#include "MasterServer.hpp"

#include <algorithm>
#include <iostream>

#include "Instance.hpp"
#include "InstanceLink.hpp"
#include "KeyboardManager.hpp"
#include "MasterLink.hpp"
#include "MasterProtocol.hpp"
#include "MouseManager.hpp"
#include "ServerManager.hpp"

namespace RType::Server {

    namespace {

        void answerRequest(ServerManager &server, InstanceManager &instances,
            const uint32_t clientId, const std::string &name)
        {
            auto known = instances.portByClient.find(clientId);
            std::optional<uint16_t> port;

            if (known != instances.portByClient.end())
                port = known->second;
            else {
                port = startInstance(instances, name.empty() ? "Lobby" : name);
                if (!port) {
                    std::cerr << "[Master] No free port for a new lobby"
                              << std::endl;
                    return;
                }
                instances.portByClient[clientId] = *port;
                std::cout << "[Master] Client " << clientId
                          << " created a lobby on port " << *port << std::endl;
            }
            try {
                MasterProtocol::sendLobbyPort(server, clientId, *port);
            } catch (const std::exception &e) {
                std::cerr << "[Master] " << e.what() << std::endl;
            }
        }

        void answerJoin(ServerManager &server, const MasterLink &link,
            const uint32_t clientId, const uint16_t port)
        {
            uint16_t answer = 0;

            for (const auto &instance : link.instances) {
                if (instance.port != port || instance.stopSent)
                    continue;
                if (instance.players < MAX_LOBBY_PLAYERS)
                    answer = port;
                break;
            }
            if (answer == 0)
                std::cout << "[Master] Client " << clientId
                          << " refused in lobby " << port << std::endl;
            try {
                MasterProtocol::sendLobbyPort(server, clientId, answer);
            } catch (const std::exception &e) {
                std::cerr << "[Master] " << e.what() << std::endl;
            }
        }

        void flushInputs(GameManager &gm)
        {
            gm.view<KeyboardManager>().each([](Entity, KeyboardManager &k) {
                k.keyPressed.clear();
                k.keyReleased.clear();
                k.keyDown.clear();
            });
            gm.view<MouseManager>().each([](Entity, MouseManager &m) {
                m.buttonPressed.clear();
                m.buttonReleased.clear();
                m.buttonDown.clear();
            });
        }

    } // namespace

    Entity createMasterServer(GameManager &gm)
    {
        const Entity master = gm.createEntity();

        gm.addComponent<ServerManager>(master, LOGIN_SERVER_PORT);
        gm.addComponent<InstanceManager>(master);
        gm.addComponent<MasterLink>(master);
        gm.addComponent<LobbyListBroadcast>(master);
        gm.createSystem<ServerManager>(ServerManager::systemServerReceived);
        gm.createSystem<>(systemMasterRequests);
        gm.createSystem<>(systemMasterLink);
        gm.createSystem<>(systemMasterLobbyList);
        /*std::cout << "[Master] Listening on port " << LOGIN_SERVER_PORT
                  << " (UDP, clients) and " << MASTER_LINK_PORT
                  << " (TCP, instances)" << std::endl;*/
        return master;
    }

    void systemMasterRequests(GameManager &gm)
    {
        gm.view<ServerManager, InstanceManager, MasterLink>().each(
            [&gm](Entity,
                ServerManager &server,
                InstanceManager &instances,
                MasterLink &link) {
                gm.view<DefaulClient, LobbyRequest>().each(
                    [&](Entity, DefaulClient &client, LobbyRequest &request) {
                        if (request.createRequested) {
                            request.createRequested = false;
                            answerRequest(server,
                                instances,
                                client.getId(),
                                request.name);
                        }
                        if (request.joinPort != 0) {
                            answerJoin(
                                server, link, client.getId(), request.joinPort);
                            request.joinPort = 0;
                        }
                    });
            });
        flushInputs(gm);
    }

    void systemMasterLobbyList(GameManager &gm)
    {
        gm.view<ServerManager,
              MasterLink,
              LobbyListBroadcast,
              InstanceManager>()
            .each([](Entity,
                      ServerManager &server,
                      MasterLink &link,
                      LobbyListBroadcast &broadcast,
                      InstanceManager &threads) {
                const auto now = std::chrono::steady_clock::now();
                std::vector<LobbyEntry> lobbies;

                for (const auto &instance : link.instances) {
                    if (instance.port == 0 || instance.stopSent)
                        continue;
                    const auto name = threads.names.find(instance.port);

                    lobbies.push_back(LobbyEntry {instance.port,
                        static_cast<uint8_t>(instance.players),
                        instance.started,
                        name != threads.names.end() ? name->second : ""});
                }
                std::sort(lobbies.begin(),
                    lobbies.end(),
                    [](const LobbyEntry &a, const LobbyEntry &b) {
                        return a.port < b.port;
                    });
                std::vector<uint8_t> payload =
                    MasterProtocol::serializeLobbyList(lobbies);

                if (payload == broadcast.lastPayload &&
                    now - broadcast.lastSent < LOBBY_LIST_BROADCAST_RATE)
                    return;
                broadcast.lastPayload = std::move(payload);
                broadcast.lastSent = now;
                try {
                    server.broadcast(
                        OpcodeRType::LOBBY_LIST, broadcast.lastPayload);
                } catch (const std::exception &e) {
                    std::cerr << "[Master] " << e.what() << std::endl;
                }
            });
    }

} // namespace RType::Server
