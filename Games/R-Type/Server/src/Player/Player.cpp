/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Player
*/

#include "Player/Player.hpp"
#include "Player/Destroy.hpp"

#include <algorithm>
#include <unordered_set>

#include "Assets.hpp"
#include "Common/Background.hpp"
#include "Common/Bullet.hpp"
#include "Common/Terrain.hpp"
#include "Config.hpp"
#include "Connexion/Lobby.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Friction2D.hpp"
#include "KeyboardManager.hpp"
#include "Player/Hud.hpp"
#include "UpdateClientEntity.hpp"
#include "Weapons/Force.hpp"
#include "Weapons/Laser.hpp"

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

    void makeInvincible(GameManager &gm, Entity player)
    {
        if (gm.hasComponent<Invincible>(player))
            gm.getComponent<Invincible>(player).remaining =
                RESPAWN_INVINCIBILITY;
        else
            gm.addComponent<Invincible>(
                player, Invincible {RESPAWN_INVINCIBILITY});
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
        state.charging = false;
        state.charge = 0.f;
        health.hp = health.maxHp;
        position = Position2D {PLAYER_START_X, WINDOW_HEIGHT / 2.f};
        gm.getComponent<Velocity2D>(player) = {};
        sheet.color.a = 255;
        sheet.currentTile = FRAME_NEUTRAL;
        makeInvincible(gm, player);
    }

    void pressFire(
        GameManager &gm, Entity entity, Player &player, const Position2D &pos)
    {
        if (player.cooldown <= 0.f) {
            // With a Force, it fires the laser instead of the ship.
            if (const auto force = findForce(gm, entity)) {
                fireLaser(gm, *force);
                player.cooldown = LASER_COOLDOWN;
            } else {
                spawnBullet(gm, entity, pos);
                player.cooldown = SHOOT_COOLDOWN;
            }
        }
        player.charging = true;
        player.charge = 0.f;
    }

    void releaseFire(
        GameManager &gm, Entity entity, Player &player, const Position2D &pos)
    {
        const int level =
            std::min(int(player.charge / CHARGE_LEVEL_TIME), CHARGE_MAX_LEVEL);

        player.charging = false;
        player.charge = 0.f;
        if (level <= 0)
            return;
        spawnChargedShot(gm, entity, pos, level);
        player.cooldown = SHOOT_COOLDOWN;
    }

    void updateCharge(
        GameManager &gm, Entity entity, Player &player, const float elapsed)
    {
        const float before = player.charge;

        if (!player.charging)
            return;
        player.charge += elapsed;
        if (before < CHARGE_LEVEL_TIME && player.charge >= CHARGE_LEVEL_TIME)
            spawnChargeEffect(gm, entity);
    }

    void playerHandle(GameManager &gm, Entity entity, Player &player,
        Velocity2D &velocity, const Position2D &pos, KeyboardManager &keyboard,
        const Health &health, const MasterClock &clock)
    {
        const float elapsed = elapsedSeconds(clock);
        const bool alive = !player.dying && health.hp > 0;

        for (const auto &key : keyboard.keyPressed) {
            setHeld(player, key, true);
            if (alive && key == Key::Value::Space)
                pressFire(gm, entity, player, pos);
        }
        for (const auto &key : keyboard.keyReleased) {
            setHeld(player, key, false);
            if (alive && key == Key::Value::Space && player.charging)
                releaseFire(gm, entity, player, pos);
        }
        keyboard.keyPressed.clear();
        keyboard.keyReleased.clear();
        if (!alive) {
            player.charging = false;
            velocity = {};
            return;
        }
        player.cooldown = std::max(0.f, player.cooldown - elapsed);
        updateCharge(gm, entity, player, elapsed);
        applyMovement(player, velocity, player.acceleration * elapsed);
    }

    void systemMasterPlayerHandle(GameManager &gm, const MasterClock &clock)
    {
        gm.view<Player, Velocity2D, Position2D, KeyboardManager, Health>().each(
            [&gm, &clock](Entity entity,
                Player &player,
                Velocity2D &velocity,
                Position2D &pos,
                KeyboardManager &keyboard,
                Health &health) {
                playerHandle(
                    gm, entity, player, velocity, pos, keyboard, health, clock);
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
            .each([&gm](Entity player,
                      Player &state,
                      Health &health,
                      Position2D &pos,
                      SpriteSheet2D &sheet,
                      UpdateClientEntity &update) {
                if (state.dying || health.hp <= 0 ||
                    gm.hasComponent<Invincible>(player))
                    return;
                const CollisionMask *shape =
                    spriteMask(gm, update.asset, sheet.currentTile);

                if (shape && touchesTerrain(gm, pos, *shape))
                    health.hp = 0;
            });
    }

    void playerInvincibility(GameManager &gm, const MasterClock &clock)
    {
        const float elapsed = elapsedSeconds(clock);

        for (auto [player, shield, sheet] :
            gm.view<Invincible, SpriteSheet2D>().getVector()) {
            shield.remaining -= elapsed;
            if (shield.remaining <= 0.f) {
                sheet.color.a = 255;
                gm.removeComponent<Invincible>(player);
                continue;
            }
            const bool dim = int(shield.remaining / INVINCIBLE_BLINK) % 2 != 0;

            sheet.color.a = dim ? INVINCIBLE_ALPHA : 255;
        }
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
                    state.lives = std::max(0, state.lives - 1);
                    sheet.color.a = 0;
                    spawnPlayerExplosion(gm, position);
                    return;
                }
                if (state.deathTimer <= 0.f)
                    return;
                state.deathTimer =
                    std::max(0.f, state.deathTimer - elapsedSeconds(clock));
                if (state.deathTimer <= 0.f && state.lives > 0)
                    respawnPlayer(gm, player);
            });
        gm.view<PlayerExplosion>().each(
            [&gm, &clock](Entity explosion, PlayerExplosion &state) {
                state.remaining -= elapsedSeconds(clock);
                if (state.remaining <= 0.f)
                    destroyNetworked(gm, explosion);
            });
    }

    std::size_t freePlayerSlot(const std::unordered_set<std::size_t> &taken)
    {
        std::size_t slot = 0;

        while (taken.contains(slot))
            ++slot;
        return slot;
    }

    void systemCreateSpaceShip(GameManager &gm)
    {
        std::unordered_set<uint32_t> connected;
        std::unordered_set<std::size_t> taken;

        if (!isGameStarted())
            return;
        for (auto [_, player] : gm.view<Player>().getVector())
            taken.insert(player.slot);
        gm.view<ServerManager>().each([&connected](Entity, ServerManager &s) {
            for (const auto &[id, _] : s.getClients())
                connected.insert(id);
        });
        for (auto [entity, defaultClient] :
            gm.view<DefaulClient>().getVector()) {
            if (defaultClient.getInit() ||
                !connected.contains(defaultClient.getId()))
                continue;
            defaultClient.init();
            const Player state {.slot = freePlayerSlot(taken)};
            taken.insert(state.slot);
            gm.addComponent<Player>(entity, state);
            spawnLifeIcons(gm, entity, state);
            spawnPlayerScore(gm, entity, state);
            gm.addComponent<Scorer>(entity);
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
                Asset::PLAYER_SHEET_1 + AssetId(state.slot % PLAYER_COLORS),
                gm,
                entity);
            makeInvincible(gm, entity);
        }
    }

} // namespace RType::Server
