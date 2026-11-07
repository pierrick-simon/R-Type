/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GravityField3D
*/

#include "GravityField3D.hpp"
#include <cmath>

namespace RType {

    void GravityField3D::gravityField3D(
        const GravityField3D &gravity, Velocity3D &velocity, float elapsed)
    {
        float len = std::sqrt(gravity.x * gravity.x + gravity.y * gravity.y +
            gravity.z * gravity.z);
        if (len == 0.f)
            return;
        float scale = gravity.g * elapsed / len;
        velocity.x += gravity.x * scale;
        velocity.y += gravity.y * scale;
        velocity.z += gravity.z * scale;
    }

    void GravityField3D::systemGravityField3D(
        const GravityField3D &gravity, Velocity3D &velocity, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        gravityField3D(gravity, velocity, elapsedSec);
    }

    void GravityField3D::systemMasterGravityField3D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, v, g] : gm.view<Velocity3D, GravityField3D>().getVector())
            gravityField3D(g, v, elapsedSec);
    }
} // namespace RType
