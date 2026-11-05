/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Laser
*/

#include "Weapons/Laser.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "Assets.hpp"
#include "Common/Terrain.hpp"
#include "Config.hpp"
#include "Player/Destroy.hpp"
#include "Player/Player.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    struct LaserShot {
        AssetId asset;
        Position2D pos;
        Velocity2D velocity;
        Hitbox hitbox;
        Projectile projectile;
        SpriteSheet2D sheet;
    };

    Entity spawnLaserShot(GameManager &gm, const LaserShot &shot, bool sound)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Bullet>(e);
        gm.addComponent<Projectile>(e, shot.projectile);
        gm.addComponent<Hitbox>(e, shot.hitbox);
        gm.addComponent<Position2D>(e, shot.pos);
        gm.addComponent<Velocity2D>(e, shot.velocity);
        gm.addComponent<SpriteSheet2D>(e, shot.sheet);
        if (!sound) {
            gm.addComponent<UpdateClientEntity>(
                e, EntityType::SPRITESHEET, shot.asset, gm, e);
            return e;
        }
        gm.addComponent<SoundInfo>(e, SoundInfo::PLAY);
        gm.addComponent<UpdateClientEntity>(e,
            EntityType::SPRITESHEET,
            shot.asset,
            gm,
            e,
            Asset::GUNSHOT_SOUND);
        return e;
    }

    void spawnMuzzle(GameManager &gm, float x, float y)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Position2D>(
            e, x - MUZZLE_WIDTH / 2.f, y - MUZZLE_HEIGHT / 2.f);
        gm.addComponent<SpriteSheet2D>(
            e, Size2D(1, MUZZLE_FRAMES), MUZZLE_FRAME_MS, LASER_SCALE);
        gm.addComponent<PlayOnce>(e);
        gm.addComponent<Lifetime>(
            e, Lifetime {MUZZLE_FRAMES * MUZZLE_FRAME_MS / 1000.f});
        gm.addComponent<UpdateClientEntity>(
            e, EntityType::SPRITESHEET, Asset::LASER_MUZZLE, gm, e);
    }

    void fireReflect(GameManager &gm, float x, float y, int level)
    {
        const float diagonal = LASER_SPEED / std::sqrt(2.f);
        const std::array<std::pair<std::size_t, Velocity2D>, 3> beams = {{
            {REFLECT_STRAIGHT, {LASER_SPEED, 0.f}},
            {REFLECT_UP, {diagonal, -diagonal}},
            {REFLECT_DOWN, {diagonal, diagonal}},
        }};
        bool sound = true;

        for (const auto &[tile, velocity] : beams) {
            for (int piece = 0; piece < level; ++piece) {
                const float back = float(piece) * REFLECT_WIDTH / velocity.x;
                const LaserShot shot {.asset = Asset::REFLECT_LASER,
                    .pos = {x - velocity.x * back,
                        y - REFLECT_HEIGHT / 2.f - velocity.y * back},
                    .velocity = velocity,
                    .hitbox = {REFLECT_WIDTH, REFLECT_HEIGHT},
                    .projectile = {REFLECT_DAMAGE, true},
                    .sheet = SpriteSheet2D(Size2D(1, REFLECT_FRAMES),
                        tile,
                        MUZZLE_FRAME_MS,
                        LASER_SCALE,
                        false)};
                const Entity e = spawnLaserShot(gm, shot, sound);

                gm.addComponent<ReflectShot>(e);
                sound = false;
            }
        }
    }

    void fireHelix(GameManager &gm, float x, float y, int level)
    {
        const LaserShot shot {.asset = Asset::HELIX_LASER,
            .pos = {x, y - HELIX_HEIGHT / 2.f},
            .velocity = {LASER_SPEED, 0.f},
            .hitbox = {.width = HELIX_WIDTH * 0.9f,
                .height = HELIX_HEIGHT * 0.8f,
                .offsetY = HELIX_HEIGHT * 0.1f},
            .projectile = {HELIX_DAMAGE[std::size_t(level - 1)], true, true},
            .sheet = SpriteSheet2D(
                Size2D(1, HELIX_FRAMES), HELIX_FRAME_MS, LASER_SCALE)};

        spawnLaserShot(gm, shot, true);
    }

    void fireRing(GameManager &gm, float x, float y, int level)
    {
        for (int ring = 0; ring < level; ++ring) {
            const float offset =
                level == 1 ? 0.f : (ring == 0 ? -RING_SPREAD : RING_SPREAD);
            const LaserShot shot {.asset = Asset::RING_LASER,
                .pos = {x - RING_WIDTH / 2.f, y + offset - RING_HEIGHT / 2.f},
                .velocity = {RING_SPEED, 0.f},
                .hitbox = {},
                .projectile = {RING_DAMAGE, true},
                .sheet = SpriteSheet2D(
                    Size2D(1, RING_FRAMES), RING_FRAME_MS, LASER_SCALE)};
            const Entity e = spawnLaserShot(gm, shot, ring == 0);

            gm.addComponent<RingShot>(e);
            gm.addComponent<PlayOnce>(e);
        }
    }

    void fireLaser(GameManager &gm, Entity force)
    {
        const auto &state = gm.getComponent<Force>(force);
        const auto &pos = gm.getComponent<Position2D>(force);
        const auto &box = gm.getComponent<Hitbox>(force);
        const float x = pos.x + box.offsetX + box.width;
        const float y = pos.y + FORCE_HEIGHT / 2.f;

        spawnMuzzle(gm, x, y);
        switch (state.laser) {
            case LaserType::Helix:
                fireHelix(gm, x, y, state.level);
                break;
            case LaserType::Ring:
                fireRing(gm, x, y, state.level);
                break;
            default:
                fireReflect(gm, x, y, state.level);
        }
    }

    bool reflectHits(GameManager &gm, float x, float y)
    {
        return y < 0.f || y + REFLECT_HEIGHT > WINDOW_HEIGHT ||
            rectTouchesTerrain(gm, x, y, REFLECT_WIDTH, REFLECT_HEIGHT);
    }

    std::size_t reflectTile(const Velocity2D &velocity)
    {
        if (velocity.y == 0.f)
            return REFLECT_STRAIGHT;
        return (velocity.x > 0.f) == (velocity.y < 0.f) ? REFLECT_UP
                                                        : REFLECT_DOWN;
    }

    void bounceReflect(GameManager &gm, ReflectShot &reflect, Position2D &pos,
        Velocity2D &velocity, SpriteSheet2D &sheet, float elapsed)
    {
        const float previousX = pos.x - velocity.x * elapsed;
        const float previousY = pos.y - velocity.y * elapsed;
        const bool offScreen = (pos.y < 0.f && velocity.y < 0.f) ||
            (pos.y + REFLECT_HEIGHT > WINDOW_HEIGHT && velocity.y > 0.f);

        if (offScreen ||
            (velocity.y != 0.f && !reflectHits(gm, pos.x, previousY))) {
            sheet.currentTile =
                velocity.y < 0.f ? REFLECT_TIP_TOP : REFLECT_TIP_BOTTOM;
            pos.y = offScreen
                ? std::clamp(pos.y, 0.f, WINDOW_HEIGHT - REFLECT_HEIGHT)
                : previousY;
            velocity.y = -velocity.y;
        } else {
            sheet.currentTile =
                velocity.x > 0.f ? REFLECT_TIP_RIGHT : REFLECT_TIP_LEFT;
            pos.x = previousX;
            velocity.x = -velocity.x;
            if (velocity.y != 0.f && reflectHits(gm, pos.x, pos.y)) {
                pos.y = previousY;
                velocity.y = -velocity.y;
            }
        }
        reflect.tipTimer = REFLECT_TIP_TIME;
        ++reflect.bounces;
    }

    void reflectSystem(GameManager &gm, const MasterClock &clock)
    {
        const float elapsed = elapsedSeconds(clock);

        for (auto [shot, reflect, pos, velocity, sheet] :
            gm.view<ReflectShot, Position2D, Velocity2D, SpriteSheet2D>()
                .getVector()) {
            if (reflectHits(gm, pos.x, pos.y)) {
                if (reflect.bounces >= REFLECT_MAX_BOUNCES) {
                    destroyNetworked(gm, shot);
                    continue;
                }
                bounceReflect(gm, reflect, pos, velocity, sheet, elapsed);
                continue;
            }
            reflect.tipTimer -= elapsed;
            if (reflect.tipTimer <= 0.f)
                sheet.currentTile = reflectTile(velocity);
        }
    }

    void ringHitbox(
        const RingShot &, const SpriteSheet2D &sheet, Hitbox &hitbox)
    {
        const auto &size = RING_SIZES[std::min(
            sheet.currentTile, std::size_t(RING_FRAMES - 1))];

        hitbox.width = size[0] * LASER_SCALE;
        hitbox.height = size[1] * LASER_SCALE;
        hitbox.offsetX = (RING_WIDTH - hitbox.width) / 2.f;
        hitbox.offsetY = (RING_HEIGHT - hitbox.height) / 2.f;
    }

} // namespace RType::Server
