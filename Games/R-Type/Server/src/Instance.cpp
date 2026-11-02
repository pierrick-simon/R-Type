/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Instance
*/

#include "Instance.hpp"

#include <atomic>
#include <exception>
#include <iostream>

#include "Assets.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "GameManager.hpp"
#include "ServerManager.hpp"
#include "Setup.hpp"

namespace RType::Server {

    namespace {

        std::atomic<bool> stopInstances = false;

        void runInstance(uint16_t port, ServerManager manager)
        {
            try {
                GameManager gm;
                AssetList assets = Asset::makeAssetList();

                gm.setAssetsMap(assets);
                const TextureCleanup cleanup {gm};
                const Entity s = createServer(gm, std::move(manager));

                std::cout << "[Instance " << port << "] Lobby started"
                          << std::endl;
                createMasterClock(gm);
                createSystems(gm, makeEnemyVisualHooks());
                auto &server = gm.getComponent<ServerManager>(s);
                gm.run(
                    60, [&] { return server.isRunning() && !stopInstances; });
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
        for (auto &thread : threads)
            if (thread.joinable())
                thread.join();
    }

    std::optional<uint16_t> startInstance(InstanceManager &instances)
    {
        const int range = LAST_INSTANCE_PORT - FIRST_INSTANCE_PORT + 1;

        for (int tries = 0; tries < range; tries++) {
            const uint16_t port = instances.nextPort;

            instances.nextPort = port >= LAST_INSTANCE_PORT
                ? FIRST_INSTANCE_PORT
                : static_cast<uint16_t>(port + 1);
            try {
                ServerManager manager(static_cast<short>(port));
                instances.threads.emplace_back(
                    runInstance, port, std::move(manager));
                return port;
            } catch (const std::exception &) {
            }
        }
        return std::nullopt;
    }

} // namespace RType::Server
