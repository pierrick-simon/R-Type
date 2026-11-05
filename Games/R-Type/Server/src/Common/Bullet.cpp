/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Bullet
*/

#include <iostream>

#include "Assets.hpp"
#include "Common/Bullet.hpp"
#include "Common/Terrain.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Player/Destroy.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    void spawnBullet(GameManager &gm, const Position2D &shooter)
    {
        const float x = shooter.x + PLAYER_WIDTH / 2;
        const float y =
            shooter.y + ((PLAYER_HEIGHT - BULLET_HEIGHT) / 2.f) + 10;
        auto bullet = gm.createEntity();

        gm.addComponent<Bullet>(bullet);
        gm.addComponent<Projectile>(bullet, Projectile {1, true});
        gm.addComponent<Hitbox>(bullet, Hitbox {BULLET_WIDTH, BULLET_HEIGHT});
        gm.addComponent<Position2D>(bullet, x, y);
        gm.addComponent<Velocity2D>(bullet, BULLET_SPEED, 0.f);
        gm.addComponent<SpriteSheet2D>(
            bullet, Size2D(1, BULLET_FRAMES), BULLET_FRAME_MS, SCALE);
        gm.addComponent<PlayOnce>(bullet);
        gm.addComponent<SoundInfo>(bullet, SoundInfo::PLAY);
        gm.addComponent<UpdateClientEntity>(bullet,
            EntityType::SPRITESHEET,
            Asset::PLAYER_BULLET,
            gm,
            bullet,
            Asset::GUNSHOT_SOUND);
    }

    void spawnChargedShot(GameManager &gm, const Position2D &shooter, int level)
    {
        const ChargedShotLevel &info = CHARGED_SHOT_LEVELS[level - 1];
        const float width = info.width * SCALE;
        const float height = info.height * SCALE;

        const Hitbox hitbox {.width = width * 0.75f,
            .height = height * 0.8f,
            .offsetX = CHARGED_SHOT_CELL_WIDTH - width * 0.75f,
            .offsetY = (CHARGED_SHOT_CELL_HEIGHT - height * 0.8f) / 2.f};
        const float x =
            shooter.x + PLAYER_WIDTH - (CHARGED_SHOT_CELL_WIDTH - width);
        const float y =
            shooter.y + PLAYER_NOSE_Y - CHARGED_SHOT_CELL_HEIGHT / 2.f;
        auto shot = gm.createEntity();

        gm.addComponent<Bullet>(shot);
        gm.addComponent<ChargedShot>(
            shot, ChargedShot {std::size_t(level - 1)});
        gm.addComponent<Projectile>(shot, Projectile {info.damage, true, true});
        gm.addComponent<Hitbox>(shot, hitbox);
        gm.addComponent<Position2D>(shot, x, y);
        gm.addComponent<Velocity2D>(shot, CHARGED_SHOT_SPEED, 0.f);
        gm.addComponent<SpriteSheet2D>(shot,
            Size2D(CHARGE_MAX_LEVEL, CHARGED_SHOT_FRAMES),
            std::size_t(level - 1) * CHARGED_SHOT_FRAMES,
            CHARGED_SHOT_FRAME_MS,
            SCALE);
        gm.addComponent<UpdateClientEntity>(shot,
            EntityType::SPRITESHEET,
            Asset::PLAYER_CHARGED_SHOT,
            gm,
            shot);
    }

    Position2D chargeEffectPosition(const Position2D &player)
    {
        return {player.x + PLAYER_WIDTH - CHARGE_OVERLAP,
            player.y + PLAYER_NOSE_Y - CHARGE_SIZE / 2.f};
    }

    void spawnChargeEffect(GameManager &gm, Entity player)
    {
        auto effect = gm.createEntity();

        gm.addComponent<ChargeEffect>(effect, ChargeEffect {player});
        gm.addComponent<Position2D>(
            effect, chargeEffectPosition(gm.getComponent<Position2D>(player)));
        gm.addComponent<SpriteSheet2D>(
            effect, Size2D(1, CHARGE_FRAMES), CHARGE_FRAME_MS, SCALE);
        gm.addComponent<UpdateClientEntity>(
            effect, EntityType::SPRITESHEET, Asset::PLAYER_CHARGE, gm, effect);
    }

    void bulletSystem(GameManager &gm)
    {
        for (auto &[bullet, tag, pos] :
            gm.view<Bullet, Position2D>().getVector()) {
            (void) tag;
            if (pos.x > WINDOW_WIDTH)
                destroyNetworked(gm, bullet);
        }
    }

    void shotTerrainSystem(GameManager &gm)
    {
        for (auto [shot, projectile, pos, box] :
            gm.view<Projectile, Position2D, Hitbox>().getVector()) {
            // The reflect laser bounces instead, see reflectSystem.
            if (gm.hasComponent<ReflectShot>(shot))
                continue;
            const bool hit = rectTouchesTerrain(gm,
                pos.x + box.offsetX,
                pos.y + box.offsetY,
                box.width,
                box.height);

            // The boss sits in the wall and the walkers shoot from the
            // floor: an enemy shot only stops once it got out of the walls.
            if (!projectile.fromPlayer &&
                !gm.hasComponent<ClearOfTerrain>(shot)) {
                if (!hit)
                    gm.addComponent<ClearOfTerrain>(shot);
                continue;
            }
            if (hit)
                destroyNetworked(gm, shot);
        }
    }

    void chargeEffectSystem(GameManager &gm)
    {
        for (auto [effect, charge, pos] :
            gm.view<ChargeEffect, Position2D>().getVector()) {
            if (!gm.hasComponent<Player>(charge.owner) ||
                !gm.getComponent<Player>(charge.owner).charging) {
                destroyNetworked(gm, effect);
                continue;
            }
            pos =
                chargeEffectPosition(gm.getComponent<Position2D>(charge.owner));
        }
    }

    void chargedShotAnimation(const ChargedShot &shot, SpriteSheet2D &sheet)
    {
        sheet.currentTile = shot.level * CHARGED_SHOT_FRAMES +
            sheet.currentTile % CHARGED_SHOT_FRAMES;
    }

} // namespace RType::Server
