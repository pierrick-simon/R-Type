/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Friction3D
*/

#ifndef FRICTION3D_HPP_
#define FRICTION3D_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"
#include "Velocity3D.hpp"

namespace RType {

    /**
     * @brief Exponential velocity damping in 3D (frame-rate independent).
     *
     * v(t + dt) = v(t) * exp(-f * dt)
     */
    struct Friction3D {
        float f = 0.f;

        static void systemFriction3D(const Friction3D &friction,
            Velocity3D &velocity, const Clock &clock);
        static void systemMasterFriction3D(
            GameManager &gm, const MasterClock &clock);
    };

} // namespace RType

#endif /* !FRICTION3D_HPP_ */
