/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySystems.hpp
*/

#ifndef ENEMYSYSTEMS_HPP_
#define ENEMYSYSTEMS_HPP_

#include "Clock.hpp"
#include "EnemyComponents.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Position2D.hpp"
#include "Velocity2D.hpp"

namespace RType {

    constexpr float PI = 3.14159265358979323846f;

    void systemEnemyPattern(
        MovementPattern &pattern, Velocity2D &velocity, const Clock &clock);

    void systemEnemyWeapon(GameManager &gm, const Enemy &enemy, Weapon &weapon,
        const Position2D &pos, const Hitbox &hitbox, const Clock &clock,
        const EnemyHooks &hooks);

    void systemEnemyDirector(GameManager &gm, EnemyDirector &director,
        const Clock &clock, const EnemyHooks &hooks);

    void systemEnemyCleanup(
        GameManager &gm, EnemyDirector &director, const Clock &clock);

    void systemEnemyCombat(
        GameManager &gm, EnemyDirector &director, const EnemyHooks &hooks);

    void registerEnemySystems(GameManager &gm, EnemyHooks hooks = {});

} // namespace RType

#endif
