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
    void GravityField2D::systemGravityField2D(
        const GravityField2D &gravity, Velocity2D &velocity, const Clock &clock)
    {
        float len = std::sqrt(gravity.x * gravity.x + gravity.y * gravity.y);
        if (len == 0.f)
            return;
        auto elapsedSec = float(clock.getElapsed().count()) / MILLI_IN_SEC;
        float scale = gravity.g * elapsedSec / len;
        velocity.x += gravity.x * scale;
        velocity.y += gravity.y * scale;
    }
}; // namespace RType
