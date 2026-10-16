/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Friction2D
*/

#ifndef FRICTION2D_HPP_
#define FRICTION2D_HPP_

#include "Clock.hpp"
#include "Velocity2D.hpp"

namespace RType {

    /**
     * @brief Friction2D of an entity in 2D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    struct Friction2D {
        float f;

        /**
         * @brief System applying friction to a velocity.
         *
         * Apply the friction to a given velocity in second
         *
         * @param friction The friction of the entity.
         * @param velocity The velocity to update.
         * @param clock The clock providing the elapsed time.
         *
         */
        static void systemFriction2D(const Friction2D &friction,
            Velocity2D &velocity, const Clock &clock);
    };

} // namespace RType

#endif /* !FRICTION2D_HPP_ */
