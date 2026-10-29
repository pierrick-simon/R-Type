/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Bullet
*/

#include <iostream>

#include "Assets.hpp"
#include "Bullet.hpp"
#include "Config.hpp"
#include "Destroy.hpp"
#include "Enemies/EnemyComponents.hpp"
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
        gm.addComponent<UpdateClientEntity>(
            bullet, EntityType::SPRITESHEET, Asset::PLAYER_BULLET, gm, bullet);
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

} // namespace RType::Server
