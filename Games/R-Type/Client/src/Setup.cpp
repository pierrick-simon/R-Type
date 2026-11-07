/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include "AssetManager.hpp"
#include "Assets.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "Decoder.hpp"
#include "Friction2D.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "Listener.hpp"
#include "Lobby.hpp"
#include "LobbyListPage.hpp"
#include "Menu.hpp"
#include "Metric.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "Sound.hpp"
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
        ::SetExitKey(KEY_NULL);
        gm.createSystem<Window>(
            [](Window &win) {
                win.setTargetFps();
                win.beginDrawing();
            },
            std::numeric_limits<int>::min());
        gm.createSystem<Window>([](Window &win) { win.endDrawing(); },
            std::numeric_limits<int>::max());
        return w;
    }

    void createClientSystems(GameManager &gm)
    {
        gm.createSystem<ClientManager>(ClientManager::systemClientReceived);
        gm.createSystem<ClientManager>(
            KeyboardProtocol::systemKeyboardProtocol);
        gm.createSystem<ClientManager>(MouseProtocol::systemMouseProtocol);
        gm.createSystem<ClientManager>(ClientManager::systemEntityCreated);
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

    Entity createMetric(GameManager &gm)
    {
        Entity m = gm.createEntity();
        gm.addComponent<Metric>(m, Asset::FONT, true);
        gm.createSystem<MasterClock>(Metric::systemUpdateMetric);
        gm.createSystem<ClientManager>(Metric::systemUpdateMetricLagometer);
        gm.createSystem<KeyboardManager>([&gm](const KeyboardManager &k) {
            bool press = false;
            for (auto key : k.keyPressed)
                if (key == Key::Value::F5) {
                    press = true;
                    break;
                }
            if (!press)
                return;
            for (auto [_, m] : gm.view<Metric>().getVector())
                m.activate();
        });
        return m;
    }

    Entity createAssetManager(GameManager &gm)
    {
        auto am = gm.createEntity();
        gm.addComponent<Assets>(am, gm, Asset::makeAssetList());
        return am;
    }

    void createSystems(GameManager &gm)
    {
        gm.createSystem<>(systemConnecting);
        gm.createSystem<Assets>(systemLobby);
        gm.createSystem<>(systemLobbyListPage);
        gm.createSystem<>(systemServerLost);
        gm.createSystem<Clock>(Clock::systemClock);
        gm.createSystem<>(systemButtons);
        gm.createSystem<>(systemTextFields);
        gm.createSystem<Assets>(systemButtonVisuals);
        gm.createSystem<>(systemMenuBackground);
        gm.createSystem<Assets>(systemDrawLobbyList);
        gm.createSystem<MasterClock>(Friction2D::systemMasterFriction2D);
        gm.createSystem<Assets>(
            systemTextAnchorManager<Font, Texture2D, Decoder>);
        gm.createSystem<Assets>(
            Sprite2D::systemDrawManagerSprite2D<Font, Texture2D, Decoder>);
        gm.createSystem<Assets>(SpriteSheet2D::
                systemDrawManagerSpriteSheet2D<Font, Texture2D, Decoder>);
        gm.createSystem<Assets>(
            Text2D::systemDrawManagerText2D<Font, Texture2D, Decoder>);
        gm.createSystem<Sound>(Sound::systemSound);
    }

    Entity createSound(GameManager &gm)
    {
        Entity s = gm.createEntity();
        gm.addComponent<Listener>(s);
        return s;
    }

} // namespace RType::Client
