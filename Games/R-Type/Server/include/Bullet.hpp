/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Bullet
*/

#ifndef BULLET_HPP_
#define BULLET_HPP_

#include "Components.hpp"
#include "GameManager.hpp"
#include "Position2D.hpp"

namespace RType {
    struct SpriteSheet2D;
} // namespace RType

namespace RType::Server {

    void spawnBullet(GameManager &gm, const Position2D &shooter);
    void spawnChargedShot(
        GameManager &gm, const Position2D &shooter, int level);
    void spawnChargeEffect(GameManager &gm, Entity player);
    void bulletSystem(GameManager &gm);
    void chargeEffectSystem(GameManager &gm);
    void chargedShotAnimation(const ChargedShot &shot, SpriteSheet2D &sheet);
} // namespace RType::Server

#endif
