/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Position2D
*/

#ifndef POSITION2D_HPP_
#define POSITION2D_HPP_

namespace RType {
    /**
     * @brief Position of an entity in 2D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    struct Position2D {
        float x = 0.f;
        float y = 0.f;

        bool operator==(const Position2D &other) const
        {
            return this->x == other.x && this->y == other.y;
        };
    };
}; // namespace RType

#endif /* !POSITION2D_HPP_ */
