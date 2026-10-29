/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>

#include "Assets.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "GameManager.hpp"
#include "ServerManager.hpp"
#include "Setup.hpp"


int main()
{
    try {
        RType::GameManager gm;
        RType::AssetList assets = RType::Asset::makeAssetList();

        gm.setAssetsMap(assets);
        const RType::Server::TextureCleanup cleanup {gm};

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
