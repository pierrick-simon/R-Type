/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace RType::Client {
    Entity createWindow(GameManager &gm)
    {
        auto w = gm.createEntity();
        gm.addComponent<Window>(w, 1920, 1080, "R-type");
        gm.createSystem<Window>(Window::systemClearWindow);
        return w;
    }

    Entity createClient(GameManager &gm)
    {
        auto c = gm.createEntity();
        gm.addComponent<ClientManager>(c, "127.0.0.1", 12345);
        gm.createSystem<ClientManager>(ClientManager::systemClientReceived);
        gm.createSystem<ClientManager>(
            KeyboardProtocol::systemKeyboardProtocol);
        gm.createSystem<ClientManager>(MouseProtocol::systemMouseProtocol);
        gm.createSystem<ClientManager>(ClientManager::systemEntityCreated);
        return c;
    }

    Entity createControl(GameManager &gm)
    {
        auto c = gm.createEntity();
        gm.addComponent<KeyboardManager>(c);
        gm.createSystem<KeyboardManager>(
            KeyboardManager::systemKeyboardManager);
        gm.addComponent<MouseManager>(c);
        gm.createSystem<MouseManager>(MouseManager::systemMouseManager);
        return c;
    }

    Entity createMasterClock(GameManager &gm)
    {
        Entity mc = gm.createEntity();
        gm.addComponent<MasterClock>(mc);
        gm.createSystem<MasterClock>(MasterClock::systemMasterClock);
        gm.createSystem<MasterClock>(Velocity2D::systemMasterVelocity2D);
        return mc;
    }

    void createSystems(GameManager &gm)
    {
        gm.createSystem<Text2D, Font, Position2D>(Text2D::systemDrawText2D);
        gm.createSystem<Sprite2D, Texture2D, Position2D>(
            Sprite2D::systemDrawSprite2D);
        gm.createSystem<SpriteSheet2D, Texture2D, Position2D>(
            SpriteSheet2D::systemDrawSpriteSheet2D);
    }
} // namespace RType::Client
