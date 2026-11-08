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

    struct SpriteSheet2D;

    EnemyHooks makeEnemyVisualHooks();
    void systemBossAnimation(const Boss &boss, SpriteSheet2D &sheet);
    void registerEnemyVisualSystems(GameManager &gm);

} // namespace RType

#endif
