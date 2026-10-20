/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>

#include "Assets.hpp"
#include "Background.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
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
        RType::Server::createWindow(gm);
        const RType::Entity keyboard = RType::Server::createKeyboardManager(gm);

        RType::Server::createBackground(gm);
        RType::Server::createSpaceShip(gm, keyboard);
        RType::createEnemyDirector(gm);
        RType::Server::createSystems(gm, RType::makeEnemyVisualHooks());

        while (!WindowShouldClose())
            gm.update();
    } catch (const RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
