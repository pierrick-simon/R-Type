/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite3D
*/

#ifndef SPRITE3D_HPP_
#define SPRITE3D_HPP_

#include "Camera3D.hpp"
#include "GameManager.hpp"
#include "Model3D.hpp"
#include "Position3D.hpp"

namespace RType {
    struct Sprite3D {
        float scale = 1;
        raylib::Color color = raylib::Color::White();

        static void systemDrawSprite3D(GameManager &gm);
    };

} // namespace RType

#endif /* !SPRITE3D_HPP_ */
