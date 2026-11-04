/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Instance
*/

#include <atomic>
#include <exception>
#include <iostream>

#include "Assets.hpp"
#include "Common/Terrain.hpp"
#include "Connexion/Instance.hpp"
#include "Connexion/InstanceLink.hpp"
#include "Connexion/Lobby.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "GameManager.hpp"
#include "Player/Player.hpp"
#include "ServerManager.hpp"
#include "Setup.hpp"

namespace RType::Server {

    namespace {

        std::atomic<bool> stopInstances = false;

        void runInstance(
            uint16_t port, ServerManager manager, const std::string &name)
        {
            try {
                GameManager gm;
                AssetList assets = Asset::makeAssetList();

                gm.setAssetsMap(assets);
                const Entity s = createServer(gm, std::move(manager));
                const Entity l = gm.createEntity();

                gm.addComponent<InstanceLink>(l, port);
                gm.createSystem<>(systemInstanceLink);
                setLobbyName(name);
                std::cout << "[Instance " << port << "] Lobby started"
                          << std::endl;
                createMasterClock(gm);
                createSystems(gm, makeEnemyVisualHooks());

                createTerrain(gm);
                createPlayerMasks(gm);
                auto &server = gm.getComponent<ServerManager>(s);
                auto &link = gm.getComponent<InstanceLink>(l);
                gm.run(60, [&] {
                    return server.isRunning() && !stopInstances &&
                        !link.stopRequested;
                });
                std::cout << "[Instance " << port << "] Stopped" << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "[Instance " << port << "] " << e.what()
                          << std::endl;
            }
        }

    } // namespace

    InstanceManager::~InstanceManager()
    {
        if (threads.empty())
            return;
        stopInstances = true;
        for (auto &[port, thread] : threads)
            if (thread.joinable())
                thread.join();
    }

    void reapInstance(InstanceManager &instances, uint16_t port)
    {
        auto it = instances.threads.find(port);

        if (it != instances.threads.end()) {
            if (it->second.joinable())
                it->second.join();
            instances.threads.erase(it);
        }
        instances.names.erase(port);
        std::erase_if(instances.portByClient,
            [port](const auto &entry) { return entry.second == port; });
    }

    std::optional<uint16_t> startInstance(
        InstanceManager &instances, const std::string &name)
    {
        const int range = LAST_INSTANCE_PORT - FIRST_INSTANCE_PORT + 1;

        for (int tries = 0; tries < range; tries++) {
            const uint16_t port = instances.nextPort;

            instances.nextPort = port >= LAST_INSTANCE_PORT
                ? FIRST_INSTANCE_PORT
                : static_cast<uint16_t>(port + 1);
            try {
                ServerManager manager(static_cast<short>(port));

                reapInstance(instances, port);
                instances.threads.emplace(port,
                    std::thread(runInstance, port, std::move(manager), name));
                instances.names[port] = name;
                return port;
            } catch (const std::exception &) {
            }
        }
        return std::nullopt;
    }

} // namespace RType::Server
