/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AngularVelocity3D
*/

#include "AngularVelocity3D.hpp"
#include <cmath>

namespace RType {

    void AngularVelocity3D::angularVelocity3D(
        const AngularVelocity3D &velocity, Rotation3D &rotation, float elapsed)
    {
        float speed = std::sqrt(velocity.x * velocity.x +
            velocity.y * velocity.y + velocity.z * velocity.z);
        if (speed == 0.f)
            return;

        Rotation3D delta = Rotation3D::fromAxisAngle(
            velocity.x, velocity.y, velocity.z, speed * elapsed);
        rotation = delta * rotation;
        rotation.normalize();
    }

    void AngularVelocity3D::systemAngularVelocity3D(
        const AngularVelocity3D &velocity, Rotation3D &rotation,
        const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        angularVelocity3D(velocity, rotation, elapsedSec);
    }

    void AngularVelocity3D::systemMasterAngularVelocity3D(
        GameManager &gm, const MasterClock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;

        for (auto [_, r, w] :
            gm.view<Rotation3D, AngularVelocity3D>().getVector())
            angularVelocity3D(w, r, elapsedSec);
    }
} // namespace RType
