/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Velocity3D
*/

#include "Velocity3D.hpp"

namespace RType {
    void Velocity3D::systemVelocity3D(
        const Velocity3D &velocity, Position3D &pos, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        pos.x += velocity.x * elapsedSec;
        pos.y += velocity.y * elapsedSec;
        pos.z += velocity.z * elapsedSec;
    }

    void Velocity3D::systemMasterVelocity3D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, pos, v] : gm.view<Position3D, Velocity3D>().getVector()) {
            pos.x += v.x * elapsedSec;
            pos.y += v.y * elapsedSec;
            pos.z += v.z * elapsedSec;
        }
    }
} // namespace RType
