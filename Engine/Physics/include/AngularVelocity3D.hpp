/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AngularVelocity3D
*/

#ifndef ANGULARVELOCITY3D_HPP_
#define ANGULARVELOCITY3D_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"
#include "Rotation3D.hpp"

namespace RType {

    /**
     * @brief Angular velocity of an entity in 3D space, in radians/second.
     *
     * (x, y, z) is the rotation axis in world space and its length is the
     * speed: (0, 3.14, 0) is a half-turn per second around the y axis.
     */
    struct AngularVelocity3D {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;

        static void systemAngularVelocity3D(const AngularVelocity3D &velocity,
            Rotation3D &rotation, const Clock &clock);
        static void systemMasterAngularVelocity3D(
            GameManager &gm, const MasterClock &clock);

    private:
        static void angularVelocity3D(const AngularVelocity3D &velocity,
            Rotation3D &rotation, float elapsed);
    };

} // namespace RType

#endif /* !ANGULARVELOCITY3D_HPP_ */
