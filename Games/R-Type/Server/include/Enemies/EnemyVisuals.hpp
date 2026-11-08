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
    SpriteSheet2D makeSheet(const SheetInfo &info);
    void systemEnemyHurtBlink(GameManager &gm);
    void systemBossAnimation(
        const Boss &boss, const Health &health, SpriteSheet2D &sheet);
    void systemBossTailAnimation(const BossTail &tail, SpriteSheet2D &sheet);
    void registerEnemyVisualSystems(GameManager &gm);

} // namespace RType

#endif
