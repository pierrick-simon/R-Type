/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Size2d
*/

#ifndef SIZE2D_HPP_
#define SIZE2D_HPP_

namespace RType {
    /**
     * @brief Size of a rectangle or dimension of a grid, in integer units.
     *
     * @warning The members are declared in the order @c height then @c width.
     *          Aggregate initialization therefore reads `{height, width}`, not
     *          `{width, height}`.
     */
    struct Size2D {
        int height = 0;
        int width = 0;

        bool operator==(const Size2D &other) const
        {
            return this->width == other.width && this->height == other.height;
        };
    };
}; // namespace RType
#endif /* !SIZE2D_HPP_ */
