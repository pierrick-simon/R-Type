/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "Assets.hpp"
#include "GameManager.hpp"
#include "Menu.hpp"
#include "Setup.hpp"
#include "Window.hpp"
#include "Sound.hpp"
#include "Texture2D.hpp"
#include "Font.hpp"

namespace {

    template<typename Component>
    void destroyAll(RType::GameManager &gm)
    {
        std::vector<RType::Entity> entities;

        gm.view<Component>().each([&entities](RType::Entity e, Component &) {
            entities.push_back(e);
        });
        for (const RType::Entity e : entities)
            gm.destroyEntity(e);
    }

    struct ResourceCleanup {
        RType::GameManager &gm;

        ~ResourceCleanup()
        {
            try {
                destroyAll<RType::Texture2D>(gm);
                destroyAll<RType::Font>(gm);
                destroyAll<RType::Sound>(gm);
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
        const ResourceCleanup cleanup {gm};
        auto w = RType::Client::createWindow(gm);
        RType::Client::createClientSystems(gm);
        auto control = RType::Client::createControl(gm);
        auto mc = RType::Client::createMasterClock(gm);

        RType::Client::createSystems(gm);
        RType::Client::createMenu(gm);
        while (!WindowShouldClose() && !RType::Client::quitRequested)
            gm.update();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
