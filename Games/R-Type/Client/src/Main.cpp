/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "AssetManager.hpp"
#include "Assets.hpp"
#include "Font.hpp"
#include "GameManager.hpp"
#include "Login.hpp"
#include "Menu.hpp"
#include "Settings.hpp"
#include "Setup.hpp"
#include "Sound.hpp"
#include "Texture2D.hpp"

int main()
{
    try {
        RType::GameManager gm;
        RType::AssetList assets = RType::Asset::makeAssetList();

        auto w = RType::Client::createWindow(gm);
        RType::Client::createClientSystems(gm);
        auto control = RType::Client::createControl(gm);
        auto mc = RType::Client::createMasterClock(gm);
        auto s = RType::Client::createSound(gm);
        auto m = RType::Client::createMetric(gm);
        auto am = RType::Client::createAssetManager(gm);

        RType::Client::createSystems(gm);
        RType::Client::loadSettings();
        RType::Client::loadSavedServer();
        RType::Client::createServerPage(gm);
        while (!RType::Client::shouldWindowClose(gm, w) &&
            !RType::Client::quitRequested)
            gm.update();
        if (gm.hasComponent<RType::Assets>(am)) {
            auto &assets = gm.getComponent<RType::Assets>(am);
            assets.unloadAll(gm);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
