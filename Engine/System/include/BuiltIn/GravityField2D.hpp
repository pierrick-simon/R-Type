/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GravityField2D
*/

#ifndef GRAVITYFIELD2D_HPP_
#define GRAVITYFIELD2D_HPP_

    #include "Velocity2D.hpp"

namespace RType {

    /**
     * @brief GravityField2D of an entity in 2D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    struct GravityField2D {
        float x = 0.f;
        float y = 1.f;
        float g = 9.80665f;

        /**
         * @brief System applying gravityField to a velocity.
         *
         * Apply the gravity to a given velocity in second
         *
         * @param gravity The gravity aplly on the entity.
         * @param velocity The velocity to update.
         * @param clock The clock providing the elapsed time.
         *
         */
        static void systemGravityField2D(const GravityField2D &gravity,
            Velocity2D &velocity, const Clock &clock);
    };

} // namespace RType

#endif /* !GRAVITYFIELD2D_HPP_ */
