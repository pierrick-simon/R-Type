/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "ClientManager.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "Sprite2D.hpp"
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
    gm.createSystem<RType::ClientManager, RType::KeyboardManager>(
        RType::KeyboardProtocol::systemKeyboardProtocol);
    gm.addComponent<RType::MouseManager>(c);
    gm.createSystem<RType::MouseManager>(
        RType::MouseManager::systemMouseManager);
    gm.createSystem<RType::ClientManager, RType::MouseManager>(
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

int main()
{
    try {
        RType::GameManager gm;
        auto w = createWindow(gm);
        auto c = createClient(gm);

        gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(
            RType::Sprite2D::systemDrawSprite2D);
        while (!WindowShouldClose())
            gm.update();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
