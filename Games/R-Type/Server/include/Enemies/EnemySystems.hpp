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

    /**
     * @brief Updates an enemy velocity from its movement pattern.
     *
     * Register this system before Velocity2D::systemVelocity2D so the velocity
     * is applied in the same frame.
     */
    void systemEnemyPattern(
        MovementPattern &pattern, Velocity2D &velocity, const Clock &clock);

    /**
     * @brief Fires a bullet every Weapon::cooldown seconds, aimed at the
     * EnemyTarget entity (straight left if there is none). Does nothing while
     * the enemy is off screen or when the target is behind it.
     */
    void systemEnemyWeapon(GameManager &gm, const Enemy &enemy, Weapon &weapon,
        const Position2D &pos, const Hitbox &hitbox, const Clock &clock,
        const EnemyHooks &hooks);

    /** Runs the director timeline: spawns the enemies at the right time. */
    void systemEnemyDirector(GameManager &gm, EnemyDirector &director,
        const Clock &clock, const EnemyHooks &hooks);

    /** Destroys enemies / bullets that left the screen and expired Lifetime. */
    void systemEnemyCleanup(
        GameManager &gm, EnemyDirector &director, const Clock &clock);

    /**
     * @brief Collisions: player bullets vs enemies (damage, score, death),
     * enemy bullets vs EnemyTarget, enemy contact vs EnemyTarget.
     */
    void systemEnemyCombat(
        GameManager &gm, EnemyDirector &director, const EnemyHooks &hooks);

    /**
     * Registers every enemy system. Call it after Clock::systemClock and
     * before Velocity2D::systemVelocity2D.
     */
    void registerEnemySystems(GameManager &gm, EnemyHooks hooks = {});

} // namespace RType

#endif /* !ENEMYSYSTEMS_HPP_ */
