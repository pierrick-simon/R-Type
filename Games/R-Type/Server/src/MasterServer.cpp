/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterServer
*/

#include "MasterServer.hpp"

#include <iostream>

#include "Instance.hpp"
#include "KeyboardManager.hpp"
#include "MasterProtocol.hpp"
#include "MouseManager.hpp"
#include "ServerManager.hpp"
#include "MasterLink.hpp"
#include "InstanceLink.hpp"

namespace RType::Server {

    namespace {

        void answerRequest(ServerManager &server, InstanceManager &instances,
            const uint32_t clientId)
        {
            auto known = instances.portByClient.find(clientId);
            std::optional<uint16_t> port;

            if (known != instances.portByClient.end())
                port = known->second;
            else {
                port = startInstance(instances);
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
        gm.createSystem<ServerManager>(ServerManager::systemServerReceived);
        gm.createSystem<>(systemMasterRequests);
        gm.createSystem<>(systemMasterLink);
        std::cout << "[Master] Listening on port " << LOGIN_SERVER_PORT
                  << " (UDP, clients) and " << MASTER_LINK_PORT
                  << " (TCP, instances)" << std::endl;
        return master;
    }

    void systemMasterRequests(GameManager &gm)
    {
        gm.view<ServerManager, InstanceManager>().each(
            [&gm](Entity, ServerManager &server, InstanceManager &instances) {
                gm.view<DefaulClient, LobbyRequest>().each(
                    [&](Entity, DefaulClient &client, LobbyRequest &request) {
                        if (!request.createRequested)
                            return;
                        request.createRequested = false;
                        answerRequest(server, instances, client.getId());
                    });
            });
        flushInputs(gm);
    }

} // namespace RType::Server
