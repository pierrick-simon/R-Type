/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#ifndef SPRITE2D_HPP_
#define SPRITE2D_HPP_

#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    struct Sprite2D {
        Origin2D origin;
        float scale = 1;
        float rotation = 0;
        bool flip = false;
        raylib::Color color = raylib::Color::White();

        static void systemSprite2D(
            Sprite2D &sprite, const Texture2D &texture, const Position2D &pos);
    };

} // namespace RType

#endif /* !SPRITE2D_HPP_ */
