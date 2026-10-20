/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Friction2D
*/

#include "Friction2D.hpp"
#include <iostream>

namespace RType {
    void Friction2D::systemFriction2D(
        const Friction2D &friction, Velocity2D &velocity, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        velocity.x *= elapsedSec * friction.f;
        velocity.y *= elapsedSec * friction.f;
    }

    void Friction2D::systemMasterFriction2D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, v, f] : gm.view<Velocity2D, Friction2D>().getVector()) {
            v.x *= elapsedSec * f.f;
            v.y *= elapsedSec * f.f;
        }
    }
}; // namespace RType
