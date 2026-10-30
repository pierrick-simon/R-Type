/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Player
*/

#include "Player.hpp"
#include "Destroy.hpp"

#include <algorithm>

#include "Assets.hpp"
#include "Background.hpp"
#include "Bullet.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Friction2D.hpp"
#include "KeyboardManager.hpp"
#include "Terrain.hpp"
#include "UpdateClientEntity.hpp"

namespace RType::Server {

    float elapsedSeconds(const MasterClock &clock)
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
                    (PLAYER_HEIGHT - PLAYER_EXPLOSION_HEIGHT * SCALE) / 2.f});
        gm.addComponent<SpriteSheet2D>(explosion,
            Size2D(1, PLAYER_EXPLOSION_FRAMES),
            PLAYER_EXPLOSION_FRAME_MS,
            SCALE);
        gm.addComponent<PlayOnce>(explosion);
        gm.addComponent<PlayerExplosion>(
            explosion, PlayerExplosion {PLAYER_DEATH_DURATION});
        gm.addComponent<UpdateClientEntity>(explosion,
            EntityType::SPRITESHEET,
            Asset::PLAYER_EXPLOSION,
            gm,
            explosion);
    }

    void respawnPlayer(GameManager &gm, Entity player)
    {
        auto &state = gm.getComponent<Player>(player);
        auto &health = gm.getComponent<Health>(player);
        auto &position = gm.getComponent<Position2D>(player);
        auto &sheet = gm.getComponent<SpriteSheet2D>(player);

        state.dying = false;
        state.deathTimer = 0.f;
        state.cooldown = 0.f;
        health.hp = health.maxHp;
        position = Position2D {PLAYER_START_X, WINDOW_HEIGHT / 2.f};
        gm.getComponent<Velocity2D>(player) = {};
        sheet.color.a = 255;
        sheet.currentTile = FRAME_NEUTRAL;
        resetEnemies(gm);
        resetBackground(gm);
    }

    void playerHandle(GameManager &gm, Player &player, Velocity2D &velocity,
        const Position2D &pos, KeyboardManager &keyboard, const Health &health,
        const MasterClock &clock)
    {
        const float elapsed = elapsedSeconds(clock);
        const bool alive = !player.dying && health.hp > 0;

        for (const auto &key : keyboard.keyPressed) {
            setHeld(player, key, true);
            if (alive && key == Key::Value::Space && player.cooldown <= 0.f) {
                spawnBullet(gm, pos);
                player.cooldown = SHOOT_COOLDOWN;
            }
        }
        for (const auto &key : keyboard.keyReleased)
            setHeld(player, key, false);
        keyboard.keyPressed.clear();
        keyboard.keyReleased.clear();
        if (!alive) {
            velocity = {};
            return;
        }
        player.cooldown = std::max(0.f, player.cooldown - elapsed);
        applyMovement(player, velocity, player.acceleration * elapsed);
    }

    void systemMasterPlayerHandle(GameManager &gm, const MasterClock &clock)
    {
        gm.view<Player, Velocity2D, Position2D, KeyboardManager, Health>().each(
            [&gm, &clock](Entity,
                Player &player,
                Velocity2D &velocity,
                Position2D &pos,
                KeyboardManager &keyboard,
                Health &health) {
                playerHandle(
                    gm, player, velocity, pos, keyboard, health, clock);
            });
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

    void createPlayerMasks(GameManager &gm)
    {
        for (AssetId sheet = Asset::PLAYER_SHEET_1;
            sheet <= Asset::PLAYER_SHEET_5;
            ++sheet)
            createSpriteMasks(gm, sheet, PLAYER_FRAMES);
    }

    void playerTerrainCollision(GameManager &gm)
    {
        gm.view<Player, Health, Position2D, SpriteSheet2D, UpdateClientEntity>()
            .each([&gm](Entity,
                      Player &state,
                      Health &health,
                      Position2D &pos,
                      SpriteSheet2D &sheet,
                      UpdateClientEntity &update) {
                if (state.dying || health.hp <= 0)
                    return;
                const CollisionMask *shape =
                    spriteMask(gm, update.asset, sheet.currentTile);

                if (shape && touchesTerrain(gm, pos, *shape))
                    health.hp = 0;
            });
    }

    void playerDeath(GameManager &gm, const MasterClock &clock)
    {
        gm.view<Player, Health, Position2D, SpriteSheet2D>().each(
            [&gm, &clock](Entity player,
                Player &state,
                Health &health,
                Position2D &position,
                SpriteSheet2D &sheet) {
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
                    respawnPlayer(gm, player);
            });
        gm.view<PlayerExplosion>().each(
            [&gm, &clock](Entity explosion, PlayerExplosion &state) {
                state.remaining -= elapsedSeconds(clock);
                if (state.remaining <= 0.f)
                    destroyNetworked(gm, explosion);
            });
    }

    void systemCreateSpaceShip(GameManager &gm)
    {
        for (auto [entity, defaultClient] :
            gm.view<DefaulClient>().getVector()) {
            if (defaultClient.getInit())
                continue;
            defaultClient.init();

            gm.addComponent<Player>(entity);
            gm.addComponent<EnemyTarget>(entity);
            gm.addComponent<Health>(entity, Health {PLAYER_HP, PLAYER_HP});
            gm.addComponent<Hitbox>(
                entity, Hitbox {PLAYER_WIDTH, PLAYER_HEIGHT});
            gm.addComponent<Position2D>(
                entity, PLAYER_START_X, WINDOW_HEIGHT / 2.f);
            gm.addComponent<Velocity2D>(entity);
            gm.addComponent<Friction2D>(entity, PLAYER_FRICTION);
            gm.addComponent<SpriteSheet2D>(entity,
                Size2D(1, PLAYER_FRAMES),
                FRAME_NEUTRAL,
                PLAYER_FRAME_MS,
                SCALE,
                false);
            gm.addComponent<UpdateClientEntity>(entity,
                EntityType::SPRITESHEET,
                Asset::PLAYER_SHEET_1,
                gm,
                entity);
        }
    }

} // namespace RType::Server
