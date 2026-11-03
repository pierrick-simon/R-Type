/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Position3D
*/

#ifndef POSITION3D_HPP_
#define POSITION3D_HPP_

namespace RType {
    /**
     * @brief Position of an entity in 3D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    struct Position3D {
        float x = 0;
        float y = 0;
        float z = 0;
    };
}; // namespace RType

#endif /* !POSITION3D_HPP_ */
