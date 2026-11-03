/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Draw3D
*/

#ifndef DRAW3D_HPP_
#define DRAW3D_HPP_

#include "Camera3D.hpp"
#include "GameManager.hpp"
#include "Window.hpp"

namespace RType {
    struct Draw3D {
        static void systemDraw3D(GameManager &gm, Window &window);
    };
} // namespace RType

#endif /* !DRAW3D_HPP_ */
