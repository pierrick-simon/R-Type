/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Laser
*/

#ifndef LASER_HPP_
#define LASER_HPP_

#include "Clock.hpp"
#include "Components.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "GameManager.hpp"

namespace RType {
    struct SpriteSheet2D;
} // namespace RType

namespace RType::Server {

    void fireLaser(GameManager &gm, Entity force);
    void reflectSystem(GameManager &gm, const MasterClock &clock);
    void ringHitbox(
        const RingShot &ring, const SpriteSheet2D &sheet, Hitbox &hitbox);
} // namespace RType::Server

#endif
