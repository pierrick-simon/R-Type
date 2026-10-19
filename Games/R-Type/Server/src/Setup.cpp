/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include "Background.hpp"
#include "Bullet.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "Friction2D.hpp"
#include "Keyboard.hpp"
#include "Player.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace RType::Server {

    Entity createWindow(GameManager &gm)
    {
        auto window = gm.createEntity();

        gm.addComponent<Window>(window, WINDOW_WIDTH, WINDOW_HEIGHT, "R-type");
        gm.createSystem<Window>(Window::systemClearWindow);
        return window;
    }

    Entity createKeyboardManager(GameManager &gm)
    {
        auto keyboardManager = gm.createEntity();

        gm.addComponent<KeyboardManager>(keyboardManager);
        gm.createSystem<KeyboardManager>(KeyboardManager::systemKeyboardManager);
        return keyboardManager;
    }

    void createSystems(GameManager &gm)
    {
        gm.createSystem<Clock>(Clock::systemClock);
        gm.createSystem<Player, Velocity2D, Position2D, Keyboard, Clock>(playerHandle);
        gm.createSystem<Friction2D, Velocity2D, Clock>(Friction2D::systemFriction2D);
        gm.createSystem<Velocity2D, Position2D, Clock>(Velocity2D::systemVelocity2D);
        gm.createSystem<Background, Position2D>(backgroundLoop);
        gm.createSystem<Player, Position2D, Velocity2D>(playerClamp);
        gm.createSystem<Player, Velocity2D, SpriteSheet2D>(playerAnimation);
        gm.createSystem<SpriteSheet2D, Clock>(SpriteSheet2D::systemAnimateSpriteSheet2D);
        gm.createSystem<>(bulletSystem);
        gm.createSystem<Sprite2D, Texture2D, Position2D>(Sprite2D::systemDrawSprite2D);
        gm.createSystem<SpriteSheet2D, Texture2D, Position2D>(SpriteSheet2D::systemDrawSpriteSheet2D);
    }

}
