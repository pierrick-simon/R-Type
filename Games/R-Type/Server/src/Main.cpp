/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <chrono>
#include <iostream>
#include <thread>

#include "Assets.hpp"
#include "Background.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ServerManager.hpp"
#include "Setup.hpp"
#include "Texture.hpp"
#include "Window.hpp"

namespace {

    struct TextureCleanup {
        RType::GameManager &gm;

        ~TextureCleanup()
        {
            try {
                RType::Server::unloadTextures(gm);
            } catch (...) {
            }
        }
    };

} // namespace

int main()
{
    try {
        RType::GameManager gm;
        RType::AssetList assets = RType::Asset::makeAssetList();

        gm.setAssetsMap(assets);
        const TextureCleanup cleanup {gm};

        auto s = RType::Server::createServer(gm);
        auto ms = RType::Server::createMasterClock(gm);
        RType::Server::createSystems(gm, RType::makeEnemyVisualHooks());
        auto &server = gm.getComponent<RType::ServerManager>(s);

        gm.run(60, [&] { return server.isRunning(); });
    } catch (const RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
