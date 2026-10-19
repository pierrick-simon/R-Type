/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySystems
*/

#include "EnemySystems.hpp"
#include "Position2D.hpp"

namespace RType::Game {

    void systemEnemyPattern(
        MovementPattern &pattern, Velocity2D &velocity, const Clock &clock)
    {
        pattern.time += float(clock.getElapsed().count()) / float(MILLI_IN_SEC);
        velocity.x = -pattern.speed;
        velocity.y = pattern.velocityY();
    }

    void systemEnemyBounds(GameManager &gm)
    {
        gm.view<Enemy, Position2D, Hitbox>().each([&gm](Entity enemy,
                                                      const Enemy &,
                                                      const Position2D &pos,
                                                      const Hitbox &box) {
            if (pos.x < -box.width * 0.5f)
                gm.destroyEntity(enemy);
        });
    }

    void systemEnemyDeath(GameManager &gm)
    {
        gm.view<Enemy, Health>().each(
            [&gm](Entity enemy, const Enemy &, const Health &health) {
                if (health.hp <= 0)
                    gm.destroyEntity(enemy);
            });
    }

    void registerEnemySystems(GameManager &gm)
    {
        gm.createSystem<MovementPattern, Velocity2D, Clock>(systemEnemyPattern);
        gm.createSystem(systemEnemyBounds);
        gm.createSystem(systemEnemyDeath);
    }

} // namespace RType::Game
