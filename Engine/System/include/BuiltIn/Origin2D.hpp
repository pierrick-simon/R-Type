/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Origin2D
*/

#ifndef ORIGIN2D_HPP_
#define ORIGIN2D_HPP_

namespace RType {
    /**
     * @brief Pivot point of a drawable, expressed in pixels.
     *
     * Rotation and scaling of sprites and texts are applied around this point.
     * (0, 0) is the top-left corner of the drawable.
     *
     * @warning Expressed in the drawable's own space (not world space) and
     *          before scaling is applied.
     */
    struct Origin2D {
        float x = 0;
        float y = 0;

        bool operator==(const Origin2D &other) const
        {
            return this->x == other.x && this->y == other.y;
        };
    };
}; // namespace RType

#endif /* !ORIGIN2D_HPP_ */
