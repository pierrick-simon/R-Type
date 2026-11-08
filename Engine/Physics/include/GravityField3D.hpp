/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GravityField3D
*/

#ifndef GRAVITYFIELD3D_HPP_
#define GRAVITYFIELD3D_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"
#include "Velocity3D.hpp"

namespace RType {

    /**
     * @brief Gravity applied to an entity in 3D space.
     *
     * (x, y, z) is the direction (does not need to be normalized),
     * g is the magnitude in units/s^2.
     * Default: pointing down (-y) since raylib's 3D uses y up.
     */
    class GravityField3D {
    public:
        float x = 0.f;
        float y = -1.f;
        float z = 0.f;
        float g = 9.80665f;

        static void systemGravityField3D(const GravityField3D &gravity,
            Velocity3D &velocity, const Clock &clock);
        static void systemMasterGravityField3D(
            GameManager &gm, const MasterClock &clock);

    private:
        static void gravityField3D(
            const GravityField3D &gravity, Velocity3D &velocity, float elapsed);
    };

} // namespace RType

#endif /* !GRAVITYFIELD3D_HPP_ */
