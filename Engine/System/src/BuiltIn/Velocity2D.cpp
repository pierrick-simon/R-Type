/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Velocity2D
*/

#include "Velocity2D.hpp"
#include <iostream>

namespace RType {
    void Velocity2D::systemVelocity2D(
        const Velocity2D &velocity, Position2D &pos, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        pos.x() += velocity.x * elapsedSec;
        pos.y() += velocity.y * elapsedSec;
    }

    void Velocity2D::systemMasterVelocity2D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, pos, v] : gm.view<Position2D, Velocity2D>().getVector()) {
            pos.x() += v.x * elapsedSec;
            pos.y() += v.y * elapsedSec;
        }
    }
}; // namespace RType
