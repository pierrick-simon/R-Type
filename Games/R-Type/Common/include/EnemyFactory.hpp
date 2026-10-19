/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyFactory
*/

#ifndef ENEMYFACTORY_HPP_
#define ENEMYFACTORY_HPP_

#include "Entity.hpp"
#include "GameComponents.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Builds enemy entities (the "archetype" of each enemy kind).
     *
     * The logic components only: no graphics, so the server can use it.
     * On the client, call attachEnemySprite() on the returned entity.
     */
    class EnemyFactory {
    public:
        /**
         * @brief Create an enemy.
         *
         * Components added: Position2D, Velocity2D, Clock, Enemy, Health,
         * Hitbox and MovementPattern.
         *
         * @param gm   The game manager.
         * @param type Which enemy to create.
         * @param x    Center of the enemy, in pixels.
         * @param y    Center of the enemy, in pixels (also the center of its
         *             vertical oscillation).
         * @return The new entity.
         *
         * @warning Outside update() the components are added immediately;
         *          inside a system they only exist at the end of the frame
         *          (see ECS_GUIDE, section 5).
         */
        static Entity spawn(GameManager &gm, EnemyType type, float x, float y);
    };

} // namespace RType::Game

#endif /* !ENEMYFACTORY_HPP_ */
