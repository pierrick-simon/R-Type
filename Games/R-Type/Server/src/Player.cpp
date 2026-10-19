/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Player
*/

#include "Player.hpp"
#include <algorithm>
#include "Bullet.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Friction2D.hpp"
#include "Texture.hpp"

namespace RType::Server {

    namespace {

        void playerMovement(Velocity2D &velocity, const float speed, const Key::Value key)
        {
            if (key == Key::Value::Q)
                velocity.x -= speed;
            if (key == Key::Value::D)
                velocity.x += speed;
            if (key == Key::Value::Z)
                velocity.y -= speed;
            if (key == Key::Value::S)
                velocity.y += speed;
        }

    }

    void playerHandle(GameManager &gm, const Player &player, Velocity2D &velocity,
        const Position2D &pos, const Keyboard &keyboard, const Clock &clock)
    {
        const auto speed = player.acceleration
            * (float(clock.getElapsed().count()) / MILLI_IN_SEC);

        for (auto &key : keyboard.manager.keyDown)
            playerMovement(velocity, speed, key);
        for (auto &key : keyboard.manager.keyPressed)
            if (key == Key::Value::Space)
                spawnBullet(gm, pos);
    }

    void playerClamp(const Player &, Position2D &pos, Velocity2D &velocity)
    {
        const float maxX = WINDOW_WIDTH - PLAYER_WIDTH;
        const float maxY = WINDOW_HEIGHT - PLAYER_HEIGHT;

        if (pos.x < 0.f || pos.x > maxX)
            velocity.x = 0.f;
        if (pos.y < 0.f || pos.y > maxY)
            velocity.y = 0.f;
        pos.x = std::clamp(pos.x, 0.f, maxX);
        pos.y = std::clamp(pos.y, 0.f, maxY);
    }

    void playerAnimation(
        const Player &, const Velocity2D &velocity, SpriteSheet2D &sheet)
    {
        if (velocity.y < -TILT_HARD)
            sheet.currentTile = FRAME_UP_MAX;
        else if (velocity.y < -TILT_SOFT)
            sheet.currentTile = FRAME_UP;
        else if (velocity.y > TILT_HARD)
            sheet.currentTile = FRAME_DOWN_MAX;
        else if (velocity.y > TILT_SOFT)
            sheet.currentTile = FRAME_DOWN;
        else
            sheet.currentTile = FRAME_NEUTRAL;
    }

    Entity createSpaceShip(GameManager &gm, Entity keyboardManager)
    {
        auto ship = gm.createEntity();

        gm.addComponent<Player>(ship);
        gm.addComponent<EnemyTarget>(ship);
        gm.addComponent<Health>(ship, Health{PLAYER_HP, PLAYER_HP});
        gm.addComponent<Hitbox>(ship, Hitbox{PLAYER_WIDTH, PLAYER_HEIGHT});
        gm.addComponent<Position2D>(ship, 100.f, WINDOW_HEIGHT / 2.f);
        gm.addComponent<Velocity2D>(ship);
        gm.addComponent<Friction2D>(ship, PLAYER_FRICTION);
        gm.addComponent<Keyboard>(
            ship, gm.getComponent<KeyboardManager>(keyboardManager));
        gm.addComponent<Texture2D>(ship, loadPixelTexture(PLAYER_PATH));
        gm.addComponent<Clock>(ship);
        gm.addComponent<SpriteSheet2D>(ship,
            Size2D(1, PLAYER_FRAMES),
            FRAME_NEUTRAL,
            PLAYER_FRAME_MS,
            std::chrono::milliseconds(PLAYER_FRAME_MS),
            std::chrono::milliseconds(0),
            Origin2D(),
            false,
            SCALE);
        return ship;
    }

}
