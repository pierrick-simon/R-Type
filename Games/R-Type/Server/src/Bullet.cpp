/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Bullet
*/

#include "Bullet.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    void spawnBullet(GameManager &gm, const Position2D &shooter)
    {
        const float x = shooter.x + PLAYER_WIDTH - 35;
        const float y =
            shooter.y + ((PLAYER_HEIGHT - BULLET_HEIGHT) / 2.f) + 10;
        auto bullet = gm.createEntity();

        gm.addComponent<Bullet>(bullet);
        gm.addComponent<Projectile>(bullet, Projectile {1, true});
        gm.addComponent<Hitbox>(bullet, Hitbox {BULLET_WIDTH, BULLET_HEIGHT});
        gm.addComponent<Position2D>(bullet, x, y);
        gm.addComponent<Velocity2D>(bullet, BULLET_SPEED, 0.f);
        gm.addComponent<Texture2D>(bullet, loadPixelTexture(BULLET_PATH));
        gm.addComponent<Clock>(bullet);
        gm.addComponent<SpriteSheet2D>(bullet,
            Size2D(1, BULLET_FRAMES),
            std::size_t(0),
            BULLET_FRAME_MS,
            std::chrono::milliseconds(BULLET_FRAME_MS),
            std::chrono::milliseconds(0),
            Origin2D(),
            true,
            SCALE);
    }

    void bulletSystem(GameManager &gm)
    {
        for (auto &[bullet, tag, sheet, pos] :
            gm.view<Bullet, SpriteSheet2D, Position2D>().getVector()) {
            (void) tag;
            if (sheet.currentTile >= BULLET_FRAMES - 1) {
                sheet.currentTile = BULLET_FRAMES - 1;
                sheet.animation = false;
            }
            if (pos.x > WINDOW_WIDTH)
                gm.destroyEntity(bullet);
        }
    }

} // namespace RType::Server
