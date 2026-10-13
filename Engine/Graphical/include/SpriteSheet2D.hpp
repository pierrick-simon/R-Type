/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#ifndef SPRITESHEET2D_HPP_
#define SPRITESHEET2D_HPP_

#include "Clock.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Size2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    struct SpriteSheet2D {
        Texture2D &texture;
        Position2D &pos;
        Origin2D &origin;
        Clock &clock;
        Size2D recNb = {1, 1};
        std::size_t currentTile = 0;
        std::chrono::milliseconds frameDuration =
            std::chrono::milliseconds(100);
        std::chrono::milliseconds sleep = std::chrono::milliseconds(0);
        float scale = 1;
        float rotation = 0;
        raylib::Color color = raylib::Color::White();

        static void systemSpriteSheet2D(SpriteSheet2D &s);
    };

} // namespace RType

#endif /* !SPRITESHEET2D_HPP_ */
