/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GravityField2D
*/

#include "GravityField2D.hpp"
#include <cmath>
#include <iostream>

namespace RType {

    void GravityField2D::gravityField2D(
        const GravityField2D &gravity, Velocity2D &velocity, float elapsed)
    {
        float len = std::sqrt(gravity.x * gravity.x + gravity.y * gravity.y);
        if (len == 0.f)
            return;
        float scale = gravity.g * elapsed / len;
        velocity.x += gravity.x * scale;
        velocity.y += gravity.y * scale;
    }

    void GravityField2D::systemGravityField2D(
        const GravityField2D &gravity, Velocity2D &velocity, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        gravityField2D(gravity, velocity, elapsedSec);
    }

    void GravityField2D::systemMasterGravityField2D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, v, g] : gm.view<Velocity2D, GravityField2D>().getVector())
            gravityField2D(g, v, elapsedSec);
    }
}; // namespace RType
