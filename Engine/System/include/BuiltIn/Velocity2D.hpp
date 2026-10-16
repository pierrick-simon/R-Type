/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Velocity2D
*/

#ifndef VELOCITY2D_HPP_
#define VELOCITY2D_HPP_

#include "Clock.hpp"
#include "Position2D.hpp"

namespace RType {

    /**
     * @brief Velocity of an entity in 2D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    struct Velocity2D {
        float x = 0;
        float y = 0;

        /**
         * @brief System updating a position.
         *
         * Apply the velocity to a given position2D in second
         *
         * @param velocity The velocity of the entity.
         * @param pos The position to update.
         * @param clock The clock providing the elapsed time.
         *
         */
        static void systemVelocity2D(
            Velocity2D &velocity, Position2D &pos, const Clock &clock);
    };

} // namespace RType

#endif /* !VELOCITY2D_HPP_ */
