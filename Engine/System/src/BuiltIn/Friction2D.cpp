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
        Friction2D &friction, Velocity2D &velocity, const Clock &clock)
    {
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        velocity.x *= elapsedSec * friction.f;
        velocity.y *= elapsedSec * friction.f;
    }
}; // namespace RType
