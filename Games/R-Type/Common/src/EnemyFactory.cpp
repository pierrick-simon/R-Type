/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyFactory
*/

#include "EnemyFactory.hpp"
#include "Clock.hpp"
#include "EnemyDefs.hpp"
#include "Position2D.hpp"
#include "Velocity2D.hpp"

namespace RType::Game {

    Entity EnemyFactory::spawn(
        GameManager &gm, EnemyType type, float x, float y)
    {
        const EnemyDef &def = enemyDef(type);
        Entity enemy = gm.createEntity();

        gm.addComponent<Position2D>(enemy, x, y);
        gm.addComponent<Velocity2D>(enemy, -def.speed, 0.f);
        gm.addComponent<Clock>(enemy);
        gm.addComponent<Enemy>(enemy, type, def.scoreValue, def.contactDamage);
        gm.addComponent<Health>(enemy, def.hp, def.hp);
        gm.addComponent<Hitbox>(enemy,
            float(def.sheet->hitW) * SPRITE_SCALE * HITBOX_RATIO,
            float(def.sheet->hitH) * SPRITE_SCALE * HITBOX_RATIO);
        gm.addComponent<MovementPattern>(
            enemy, def.pattern, def.speed, def.amplitude, def.period, 0.f);
        return enemy;
    }

} // namespace RType::Game
