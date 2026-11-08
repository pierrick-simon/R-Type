/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Velocity3D
*/

#ifndef VELOCITY3D_HPP_
#define VELOCITY3D_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"
#include "Position3D.hpp"

namespace RType {

    /**
     * @brief Velocity of an entity in 3D space, in units per second.
     *
     * Follows raylib's 3D convention: right-handed, y is up.
     */
    struct Velocity3D {
        float x = 0;
        float y = 0;
        float z = 0;

        /**
         * @brief System updating a position.
         *
         * @param velocity The velocity of the entity.
         * @param pos The position to update.
         * @param clock The clock providing the elapsed time.
         */
        static void systemVelocity3D(
            const Velocity3D &velocity, Position3D &pos, const Clock &clock);
        static void systemMasterVelocity3D(
            GameManager &gm, const MasterClock &clock);
    };

} // namespace RType

#endif /* !VELOCITY3D_HPP_ */
