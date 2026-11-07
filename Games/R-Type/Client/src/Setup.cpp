/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include <algorithm>
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "FinishingSound.hpp"
#include "Friction2D.hpp"
#include "GameOver.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "Listener.hpp"
#include "Lobby.hpp"
#include "LobbyListPage.hpp"
#include "Login.hpp"
#include "Menu.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"
#include "Settings.hpp"

namespace RType::Client {
    namespace {
        constexpr float GAME_WIDTH = 1920.f;
        constexpr float GAME_HEIGHT = 1080.f;

        ::Camera2D fitGameToWindow()
        {
            const auto width = float(::GetScreenWidth());
            const auto height = float(::GetScreenHeight());
            const float zoom =
                std::min(width / GAME_WIDTH, height / GAME_HEIGHT);
            const ::Vector2 offset = {(width - GAME_WIDTH * zoom) / 2.f,
                (height - GAME_HEIGHT * zoom) / 2.f};

            ::SetMouseOffset(int(-offset.x), int(-offset.y));
            ::SetMouseScale(1.f / zoom, 1.f / zoom);
            return {offset, {0.f, 0.f}, 0.f, zoom};
        }
    } // namespace

    Entity createWindow(GameManager &gm)
    {
        auto w = gm.createEntity();
        gm.addComponent<Window>(w, int(GAME_WIDTH), int(GAME_HEIGHT), "R-type");
        ::SetExitKey(KEY_NULL);
        gm.createSystem<Window>([](Window &win) {
            win.setTargetFps();
            win.beginDrawing();
            ::BeginMode2D(fitGameToWindow());
        });
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

    void createSystems(GameManager &gm)
    {
        gm.createSystem<>(systemConnecting);
        gm.createSystem<>(systemLobby);
        gm.createSystem<>(systemGameOver);
        gm.createSystem<>(systemLobbyListPage);
        gm.createSystem<>(systemLogin);
        gm.createSystem<>(systemServerLost);
        gm.createSystem<Clock>(Clock::systemClock);
        gm.createSystem<>(systemButtons);
        gm.createSystem<>(systemTextFields);
        gm.createSystem<>(systemButtonVisuals);
        gm.createSystem<>(systemMenuBackground);
        gm.createSystem<Text2D, Font, TextAnchor>(systemTextAnchor);
        gm.createSystem<MasterClock>(Friction2D::systemMasterFriction2D);
        gm.createSystem<Sprite2D, Texture2D, Position2D>(
            Sprite2D::systemDrawSprite2D);
        gm.createSystem<>(systemDrawLobbyList);
        gm.createSystem<>(systemDrawLoginPopup);
        gm.createSystem<>(systemDrawSettings);
        gm.createSystem<SpriteSheet2D, Texture2D, Position2D>(
            SpriteSheet2D::systemDrawSpriteSheet2D);
        gm.createSystem<Text2D, Font, Position2D>(Text2D::systemDrawText2D);
        gm.createSystem<>(systemDrawPasswordFields);
        gm.createSystem<>(systemSettings);
        gm.createSystem<>(systemVolume);
        gm.createSystem<Sound>(Sound::systemSound);
        gm.createSystem<Window>([](Window &win) {
            ::EndMode2D();
            win.endDrawing();
        });
    }

    Entity createSound(GameManager &gm)
    {
        Entity s = gm.createEntity();
        gm.addComponent<Listener>(s);
        return s;
    }

} // namespace RType::Client
