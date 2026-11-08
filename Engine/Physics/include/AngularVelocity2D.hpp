/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AngularVelocity2D
*/

#ifndef ANGULARVELOCITY2D_HPP_
#define ANGULARVELOCITY2D_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"
#include "Rotation2D.hpp"

namespace RType {

    /**
     * @brief Angular velocity of an entity in 2D space, in radians/second.
     *
     * Positive is clockwise on screen (y down).
     */
    struct AngularVelocity2D {
        float w = 0.f;

        /**
         * @brief System updating a rotation.
         *
         * @param velocity The angular velocity of the entity.
         * @param rotation The rotation to update.
         * @param clock The clock providing the elapsed time.
         */
        static void systemAngularVelocity2D(const AngularVelocity2D &velocity,
            Rotation2D &rotation, const Clock &clock);
        static void systemMasterAngularVelocity2D(
            GameManager &gm, const MasterClock &clock);

    private:
        static void angularVelocity2D(const AngularVelocity2D &velocity,
            Rotation2D &rotation, float elapsed);
    };

} // namespace RType

#endif /* !ANGULARVELOCITY2D_HPP_ */
