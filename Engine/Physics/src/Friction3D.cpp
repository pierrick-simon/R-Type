/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Friction3D
*/

#include <cmath>

#include "Friction3D.hpp"

namespace RType {
    void Friction3D::systemFriction3D(
        const Friction3D &friction, Velocity3D &velocity, const Clock &clock)
    {
        const float dt =
            static_cast<float>(clock.getElapsed().count()) / MILLI_IN_SEC;
        const float k = std::exp(-friction.f * dt);
        velocity.x *= k;
        velocity.y *= k;
        velocity.z *= k;
    }

    void Friction3D::systemMasterFriction3D(
        GameManager &gm, const MasterClock &clock)
    {
        const float dt = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, v, f] : gm.view<Velocity3D, Friction3D>().getVector()) {
            const float k = std::exp(-f.f * dt);
            v.x *= k;
            v.y *= k;
            v.z *= k;
        }
    }
} // namespace RType
