/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySprites
*/

#ifndef ENEMYSPRITES_HPP_
#define ENEMYSPRITES_HPP_

#include "Entity.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Give a look to an enemy created by EnemyFactory (client only).
     *
     * Adds a Texture2D and a SpriteSheet2D matching the enemy type. The
     * sprite is centered on the entity position and scaled by SPRITE_SCALE.
     * The assets are looked up relative to the working directory, so run
     * the client from the root of the repository.
     *
     * @param gm    The game manager.
     * @param enemy An entity that has the Enemy component.
     *
     * @throw EngineException If the texture cannot be loaded.
     *
     * @warning The window must be created first (OpenGL context), see
     *          Texture2D.
     * @warning Each call loads the PNG again: Texture2D is owned by the
     *          entity (no texture sharing in the engine yet).
     */
    void attachEnemySprite(GameManager &gm, Entity enemy);

} // namespace RType::Game

#endif /* !ENEMYSPRITES_HPP_ */
