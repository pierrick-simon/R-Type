/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <algorithm>
#include <iostream>

#include "Friction2D.hpp"
#include "GameManager.hpp"
#include "Keyboard.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"
#include "Sprite2D.hpp"

#define BULLETSPEED 1000
struct Player {
    float speed = 3000;
};

struct Bullet {
    float speed = BULLETSPEED;
};

void playerMovement(RType::Velocity2D &v, const float s, const RType::Key::Value key)
{
    if (key == RType::Key::Value::Q)
        v.x -= s;
    if (key == RType::Key::Value::D)
        v.x += s;
    if (key == RType::Key::Value::Z)
        v.y -= s;
    if (key == RType::Key::Value::S)
        v.y += s;
}

void playerShoot(RType::GameManager &gm, const RType::Position2D &pos, const RType::Key::Value key)
{
    if (key != RType::Key::Value::Space)
        return;
    auto b = gm.createEntity();
    gm.addComponent<Bullet>(b);
    gm.addComponent<RType::Position2D>(b, pos.x, pos.y);
    gm.addComponent<RType::Velocity2D>(b, BULLETSPEED, 0);
    gm.addComponent<RType::Texture2D>(b, "R-Type/sprites/bullet.png");
    gm.addComponent<RType::Sprite2D>(b, RType::Origin2D());
    gm.addComponent<RType::Clock>(b);
}

void playerHandle(RType::GameManager &gm, const Player &p, RType::Velocity2D &v, const RType::Position2D &pos,
    const RType::Keyboard &k, const RType::Clock &c)
{
    const auto s =
        p.speed * (float(c.getElapsed().count()) / RType::MILLI_IN_SEC);
    for (auto &key : k.manager.keyDown)
        playerMovement(v, s, key);
    for (auto &key : k.manager.keyPressed)
        playerShoot(gm, pos, key);
}

void playerAnimation(
    const Player &p, const RType::Velocity2D &v, RType::SpriteSheet2D &ss)
{
    if (v.y < -100)
        ss.currentTile = 13;
    else if (v.y < -40)
        ss.currentTile = 14;
    else if (v.y > 100)
        ss.currentTile = 9;
    else if (v.y > 40)
        ss.currentTile = 8;
    else
        ss.currentTile = 15;
}

RType::Entity createWindow(RType::GameManager &gm)
{
    auto w = gm.createEntity();
    gm.addComponent<RType::Window>(w, 1920, 1080, "R-type");
    gm.createSystem<RType::Window>(RType::Window::systemClearWindow);
    return w;
}

RType::Entity createKeyboardManager(RType::GameManager &gm)
{
    auto km = gm.createEntity();
    gm.addComponent<RType::KeyboardManager>(km);
    gm.createSystem<RType::KeyboardManager>(
        RType::KeyboardManager::systemKeyboardManager);
    return km;
}

RType::Entity createSpaceShip(RType::GameManager &gm, RType::Entity km)
{
    auto s = gm.createEntity();
    gm.addComponent<Player>(s);
    gm.addComponent<RType::Position2D>(s, 0, 100);
    gm.addComponent<RType::Velocity2D>(s);
    gm.addComponent<RType::Friction2D>(s, 120);
    gm.addComponent<RType::Keyboard>(
        s, gm.getComponent<RType::KeyboardManager>(km));
    gm.addComponent<RType::Texture2D>(s, "R-Type/sprites/r-typesheet5.gif");
    gm.addComponent<RType::Clock>(s);
    gm.addComponent<RType::SpriteSheet2D>(s, RType::Size2D(1, 16), 0, 200);
    gm.getComponent<RType::SpriteSheet2D>(s).animation = false;
    gm.createSystem<RType::Sprite2D, RType::Texture2D, RType::Position2D>(RType::Sprite2D::systemDrawSprite2D);
    return s;
}

int main()
{
    try {
        RType::GameManager gm;
        auto w = createWindow(gm);
        auto km = createKeyboardManager(gm);
        auto s = createSpaceShip(gm, km);
        gm.createSystem<RType::Clock>(RType::Clock::systemClock);
        gm.createSystem<Player,
            RType::Velocity2D,
            RType::Position2D,
            RType::Keyboard,
            RType::Clock>(playerHandle);
        gm.createSystem<Player, RType::Velocity2D, RType::SpriteSheet2D>(
            playerAnimation);
        gm.createSystem<RType::SpriteSheet2D,
            RType::Texture2D,
            RType::Position2D>(RType::SpriteSheet2D::systemDrawSpriteSheet2D);
        gm.createSystem<RType::Friction2D, RType::Velocity2D, RType::Clock>(
            RType::Friction2D::systemFriction2D);
        gm.createSystem<RType::Velocity2D, RType::Position2D, RType::Clock>(
            RType::Velocity2D::systemVelocity2D);

        while (!WindowShouldClose())
            gm.update();
    } catch (RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
