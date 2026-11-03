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
#include "Texture2D.hpp"
#include "Window.hpp"

int main()
{
    try {
        RType::GameManager gm;
        RType::AssetList assets = RType::Asset::makeAssetList();
        gm.setAssetsMap(assets);
        auto w = RType::Client::createWindow(gm);
        auto c = RType::Client::createClient(gm);
        auto control = RType::Client::createControl(gm);
        auto mc = RType::Client::createMasterClock(gm);
        auto s = RType::Client::createSound(gm);

        RType::Client::createSystems(gm);
        while (!WindowShouldClose())
            gm.update();
        RType::Texture2D::unloadTextures2D(gm);
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
