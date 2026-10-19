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
#include "Friction2D.hpp"
#include "Texture.hpp"

namespace RType::Server {

    namespace {

        float elapsedSeconds(const Clock &clock)
        {
            return static_cast<float>(clock.getElapsed().count()) /
                static_cast<float>(MILLI_IN_SEC);
        }

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

        void applyMovement(
            const Player &player, Velocity2D &velocity, const float speed)
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

        void spawnPlayerExplosion(GameManager &gm, const Position2D &position)
        {
            const Entity explosion = gm.createEntity();

            gm.addComponent<Position2D>(explosion,
                Position2D {position.x +
                        (PLAYER_WIDTH - PLAYER_EXPLOSION_WIDTH * SCALE) / 2.f,
                    position.y +
                        (PLAYER_HEIGHT - PLAYER_EXPLOSION_HEIGHT * SCALE) /
                            2.f});
            gm.addComponent<Texture2D>(
                explosion, loadPixelTexture(PLAYER_EXPLOSION_PATH));
            gm.addComponent<SpriteSheet2D>(explosion,
                Size2D {1, PLAYER_EXPLOSION_FRAMES},
                0,
                PLAYER_EXPLOSION_FRAME_MS,
                std::chrono::milliseconds(PLAYER_EXPLOSION_FRAME_MS),
                std::chrono::milliseconds(0),
                Origin2D {},
                true,
                SCALE);
            gm.addComponent<Clock>(explosion);
            gm.addComponent<PlayerExplosion>(
                explosion, PlayerExplosion {PLAYER_DEATH_DURATION});
        }

    } // namespace

    void playerHandle(GameManager &gm, Player &player, Velocity2D &velocity,
        const Position2D &pos, const Keyboard &keyboard, const Clock &clock,
        const Health &health)
    {
        if (player.dying || health.hp <= 0) {
            velocity = {};
            return;
        }
        const float elapsed = elapsedSeconds(clock);
        const float speed = player.acceleration * elapsed;

        player.cooldown = std::max(0.f, player.cooldown - elapsed);
        for (const auto &key : keyboard.manager.keyPressed) {
            setHeld(player, key, true);
            if (key == Key::Value::Space && player.cooldown <= 0.f) {
                spawnBullet(gm, pos);
                player.cooldown = SHOOT_COOLDOWN;
            }
        }
        for (const auto &key : keyboard.manager.keyReleased)
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

    void playerDeath(GameManager &gm)
    {
        gm.view<Player, Health, Position2D, SpriteSheet2D, Clock>().each(
            [&gm](Entity player,
                Player &state,
                Health &health,
                Position2D &position,
                SpriteSheet2D &sheet,
                Clock &clock) {
                if (health.hp > 0)
                    return;
                if (!state.dying) {
                    state.dying = true;
                    state.deathTimer = PLAYER_DEATH_DURATION;
                    sheet.color.a = 0;
                    spawnPlayerExplosion(gm, position);
                    return;
                }
                state.deathTimer -= elapsedSeconds(clock);
                if (state.deathTimer <= 0.f)
                    gm.destroyEntity(player);
            });
        gm.view<PlayerExplosion, Clock>().each(
            [&gm](Entity explosion, PlayerExplosion &state, Clock &clock) {
                state.remaining -= elapsedSeconds(clock);
                if (state.remaining <= 0.f)
                    gm.destroyEntity(explosion);
            });
    }

    Entity createSpaceShip(GameManager &gm, Entity keyboardManager)
    {
        const Entity ship = gm.createEntity();

        gm.addComponent<Player>(ship);
        gm.addComponent<EnemyTarget>(ship);
        gm.addComponent<Health>(ship, Health {PLAYER_HP, PLAYER_HP});
        gm.addComponent<Hitbox>(ship, Hitbox {PLAYER_WIDTH, PLAYER_HEIGHT});
        gm.addComponent<Position2D>(ship, 100.f, WINDOW_HEIGHT / 2.f);
        gm.addComponent<Velocity2D>(ship);
        gm.addComponent<Friction2D>(ship, PLAYER_FRICTION);
        gm.addComponent<Keyboard>(
            ship, gm.getComponent<KeyboardManager>(keyboardManager));
        gm.addComponent<Texture2D>(ship, loadPixelTexture(PLAYER_PATH));
        gm.addComponent<Clock>(ship);
        gm.addComponent<SpriteSheet2D>(ship,
            Size2D {1, PLAYER_FRAMES},
            FRAME_NEUTRAL,
            PLAYER_FRAME_MS,
            std::chrono::milliseconds(PLAYER_FRAME_MS),
            std::chrono::milliseconds(0),
            Origin2D {},
            false,
            SCALE);
        return ship;
    }

} // namespace RType::Server
