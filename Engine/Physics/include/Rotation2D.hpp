/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Rotation2D
*/

#ifndef ROTATION2D_HPP_
#define ROTATION2D_HPP_

namespace RType {

    /**
     * @brief Orientation of an entity in 2D space, in radians.
     *
     * Follows raylib's 2D convention: y grows downward, so a positive angle
     * is clockwise on screen. Kept in [0, 2*PI).
     * raylib's draw functions expect degrees: use toDegrees().
     */
    struct Rotation2D {
        float angle = 0.f;

        static constexpr float MY_PI = 3.14159265358979323846f;

        [[nodiscard]] float toDegrees() const
        {
            return angle * 180.f / MY_PI;
        }

        static Rotation2D fromDegrees(float deg)
        {
            return Rotation2D {deg * MY_PI / 180.f};
        }
    };

} // namespace RType

#endif /* !ROTATION2D_HPP_ */
