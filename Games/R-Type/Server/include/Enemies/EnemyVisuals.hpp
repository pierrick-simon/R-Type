/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.hpp
*/

#ifndef ENEMYVISUALS_HPP_
#define ENEMYVISUALS_HPP_

#include "EnemyManager.hpp"

namespace RType {

    /**
     * @brief Hooks adding the textures / sprite sheets to enemies, enemy
     * bullets and explosions. Keeps the raylib dependency out of the enemy
     * logic (and out of the unit tests).
     *
     * @warning The Window must exist before the first enemy spawns, since
     *          loading a Texture2D needs the OpenGL context.
     */
    EnemyHooks makeEnemyVisualHooks();

} // namespace RType

#endif /* !ENEMYVISUALS_HPP_ */
