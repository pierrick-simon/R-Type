/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "Sprite2D.hpp"
#include "Text2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

RType::Entity createWindow(RType::GameManager &gm)
{
    auto w = gm.createEntity();
    gm.addComponent<RType::Window>(w, 800, 600, "R-type");
    gm.createSystem<RType::Window>(RType::Window::systemClearWindow);
    return w;
}

RType::Entity createClient(RType::GameManager &gm)
{
    auto c = gm.createEntity();
    gm.addComponent<RType::ClientManager>(c, "127.0.0.1", 12345);
    gm.createSystem<RType::ClientManager>(
        RType::ClientManager::systemClientReceived);
    gm.addComponent<RType::KeyboardManager>(c);
    gm.createSystem<RType::KeyboardManager>(
        RType::KeyboardManager::systemKeyboardManager);
    gm.createSystem<RType::ClientManager>(
        RType::KeyboardProtocol::systemKeyboardProtocol);
    gm.createSystem<RType::ClientManager>(
        RType::MouseProtocol::systemMouseProtocol);
    return c;
}

RType::Entity createKeyboard(RType::GameManager &gm)
{
    auto k = gm.createEntity();
    gm.addComponent<RType::KeyboardManager>(k);
    gm.createSystem<RType::KeyboardManager>(
        RType::KeyboardManager::systemKeyboardManager);
    return k;
}

RType::Entity createMouse(RType::GameManager &gm)
{
    auto k = gm.createEntity();
    gm.addComponent<RType::MouseManager>(k);
    gm.createSystem<RType::MouseManager>(
        RType::MouseManager::systemMouseManager);
    return k;
}

RType::Entity createMasterClock(RType::GameManager &gm)
{
    RType::Entity mc = gm.createEntity();
    gm.addComponent<RType::MasterClock>(mc);
    gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);
    gm.createSystem<RType::MasterClock>(
        RType::Velocity2D::systemMasterVelocity2D);
    return mc;
}

int main()
{
    try {
        RType::GameManager gm;
        auto w = createWindow(gm);
        auto c = createClient(gm);
        auto k = createKeyboard(gm);
        auto m = createMouse(gm);
        auto mc = createMasterClock(gm);

        std::unordered_map<int, std::string> map = {
            {0, "../loading.png"}, {1, "../font.ttf"}};
        gm.setAssetsMap(map);
        gm.createSystem<RType::Text2D, RType::Font, RType::Position2D>(
            RType::Text2D::systemDrawText2D);
        gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(
            RType::Sprite2D::systemDrawSprite2D);
        while (!WindowShouldClose())
            gm.update();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
