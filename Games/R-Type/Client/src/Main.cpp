/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "Assets.hpp"
#include "GameManager.hpp"
#include "Setup.hpp"
#include "Texture.hpp"
#include "Window.hpp"

namespace {

    struct TextureCleanup {
        RType::GameManager &gm;

        ~TextureCleanup()
        {
            try {
                RType::Client::unloadTextures(gm);
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
        auto w = RType::Client::createWindow(gm);
        auto c = RType::Client::createClient(gm);
        auto control = RType::Client::createControl(gm);
        auto mc = RType::Client::createMasterClock(gm);
        auto s = RType::Client::createSound(gm);

        RType::Client::createSystems(gm);
        while (!WindowShouldClose())
            gm.update();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
