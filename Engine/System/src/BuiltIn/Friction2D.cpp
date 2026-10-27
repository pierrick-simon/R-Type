/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Friction2D
*/

#include <cmath>
#include <iostream>

#include "Friction2D.hpp"

namespace RType {
    void Friction2D::systemFriction2D(
        const Friction2D &friction, Velocity2D &velocity, const Clock &clock)
    {
        const float dt =
            static_cast<float>(clock.getElapsed().count()) / MILLI_IN_SEC;
        const float k = std::exp(-friction.f * dt);
        velocity.x *= k;
        velocity.y *= k;
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
