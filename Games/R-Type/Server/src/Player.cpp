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

    void setHeld(Player &player, const Key::Value key, const bool held)
    {
        if (key == Key::Value::Q)
            player.left = held;
        if (key == Key::Value::D)
            player.right = held;
        if (key == Key::Value::Z)
            player.up = held;
        if (key == Key::Value::S)
            player.down = held;
    }

    void applyMovement(const Player &player, Velocity2D &velocity, const float speed)
    {
        if (player.left)
            velocity.x -= speed;
        if (player.right)
            velocity.x += speed;
        if (player.up)
            velocity.y -= speed;
        if (player.down)
            velocity.y += speed;
    }

    void playerHandle(GameManager &gm, Player &player, Velocity2D &velocity,
        const Position2D &pos, const Keyboard &keyboard, const Clock &clock)
    {
        const float elapsed = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        const float speed = player.acceleration * elapsed;

        player.cooldown = std::max(0.f, player.cooldown - elapsed);
        for (auto &key : keyboard.manager.keyPressed) {
            setHeld(player, key, true);
            if (key == Key::Value::Space && player.cooldown <= 0.f) {
                spawnBullet(gm, pos);
                player.cooldown = SHOOT_COOLDOWN;
            }
        }
        for (auto &key : keyboard.manager.keyReleased)
            setHeld(player, key, false);
        applyMovement(player, velocity, speed);
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
