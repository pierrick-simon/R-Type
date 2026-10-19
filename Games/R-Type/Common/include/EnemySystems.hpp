/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySystems
*/

#ifndef ENEMYSYSTEMS_HPP_
#define ENEMYSYSTEMS_HPP_

#include "Clock.hpp"
#include "GameComponents.hpp"
#include "GameManager.hpp"
#include "Velocity2D.hpp"

namespace RType::Game {

    /**
     * @brief System making an enemy follow its MovementPattern.
     *
     * Advances the pattern clock and writes the velocity of the enemy.
     * The actual displacement is done by Velocity2D::systemVelocity2D.
     */
    void systemEnemyPattern(
        MovementPattern &pattern, Velocity2D &velocity, const Clock &clock);

    /**
     * @brief Frame system destroying the enemies that left the screen
     *        (on the left side).
     */
    void systemEnemyBounds(GameManager &gm);

    /**
     * @brief Frame system destroying the enemies without hit points.
     */
    void systemEnemyDeath(GameManager &gm);

    /**
     * @brief Register the enemy systems. Call it ONCE, at startup.
     *
     * @warning The caller must have registered, BEFORE this call, the generic
     *          engine systems the enemies rely on, once each:
     *          `createSystem<Clock>(Clock::systemClock)` and
     *          `createSystem<Velocity2D, Position2D, Clock>(
     *          Velocity2D::systemVelocity2D)`.
     *          Registering them here would update the clocks twice per
     *          frame as soon as the players also use them.
     */
    void registerEnemySystems(GameManager &gm);

} // namespace RType::Game

#endif /* !ENEMYSYSTEMS_HPP_ */
