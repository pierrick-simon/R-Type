/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AngularVelocity2D
*/

#include "AngularVelocity2D.hpp"
#include <cmath>

namespace RType {

    void AngularVelocity2D::angularVelocity2D(
        const AngularVelocity2D &velocity, Rotation2D &rotation, float elapsed)
    {
        constexpr float twoPi = 2.f * Rotation2D::MY_PI;

        rotation.angle =
            std::fmod(rotation.angle + velocity.w * elapsed, twoPi);
        if (rotation.angle < 0.f)
            rotation.angle += twoPi;
    }

    void AngularVelocity2D::systemAngularVelocity2D(
        const AngularVelocity2D &velocity, Rotation2D &rotation,
        const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        angularVelocity2D(velocity, rotation, elapsedSec);
    }

    void AngularVelocity2D::systemMasterAngularVelocity2D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, r, w] :
            gm.view<Rotation2D, AngularVelocity2D>().getVector())
            angularVelocity2D(w, r, elapsedSec);
    }
} // namespace RType
