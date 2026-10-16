/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>

#include "GameManager.hpp"
#include "Window.hpp"
#include "Texture2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Velocity2D.hpp"
#include "Keyboard.hpp"

struct Player {
    float speed = 10;
};

void playerMovement(Player &p, RType::Velocity2D &v, RType::Keyboard &k)
{
    for (auto &key : k.keyDown) {
        std::cout << static_cast<int>(key) << std::endl;
        if (key == RType::Key::Value::Q)
            v.x -= p.speed;
        if (key == RType::Key::Value::D)
            v.x += p.speed;
        if (key == RType::Key::Value::Z)
            v.y -= p.speed;
        if (key == RType::Key::Value::S)
            v.y += p.speed;
    }
}

RType::Entity createWindow(RType::GameManager &gm)
{
    auto w = gm.createEntity();
    gm.addComponent<RType::Window>(w);
    gm.createSystem<RType::Window>(RType::Window::systemWindow);
    return w;
}

RType::Entity createSpaceShip(RType::GameManager &gm)
{
    auto s = gm.createEntity();
    gm.addComponent<Player>(s);
    gm.addComponent<RType::Position2D>(s, 0, 100);
    gm.addComponent<RType::Velocity2D>(s);
    gm.addComponent<RType::Keyboard>(s);
    gm.addComponent<RType::Texture2D>(s, "R-Type/sprites/r-typesheet5.gif");
    gm.addComponent<RType::Clock>(s);
    gm.addComponent<RType::SpriteSheet2D>(s, RType::Size2D(1, 16), 0, 200);
    return s;
}

int main()
{
    try {
        RType::GameManager gm;
        auto w = createWindow(gm);
        auto s = createSpaceShip(gm);
        gm.createSystem<RType::Clock>(RType::Clock::systemClock);
        gm.createSystem<RType::SpriteSheet2D, RType::Texture2D, RType::Position2D, RType::Clock>(RType::SpriteSheet2D::systemSpriteSheet2D);
        gm.createSystem<RType::Keyboard>(RType::Keyboard::systemKeyboard);
        gm.createSystem<Player, RType::Velocity2D, RType::Keyboard>(playerMovement);
        gm.createSystem<RType::Velocity2D, RType::Position2D, RType::Clock>(RType::Velocity2D::systemVelocity2D);

        while (!WindowShouldClose())
            gm.update();
    } catch (RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
