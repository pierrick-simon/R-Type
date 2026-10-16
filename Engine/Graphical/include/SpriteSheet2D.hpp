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
        Size2D recNb = {1, 1};
        std::size_t currentTile = 0;
        std::chrono::milliseconds frameDuration =
            std::chrono::milliseconds(100);
        std::chrono::milliseconds sleep = std::chrono::milliseconds(0);
        Origin2D origin;
        bool animation = true;
        float scale = 1;
        float rotation = 0;
        bool flip = false;
        raylib::Color color = raylib::Color::White();

        static void systemSpriteSheet2D(SpriteSheet2D &sprite,
            const Texture2D &texture, const Position2D &pos,
            const Clock &clock);
    };

} // namespace RType

#endif /* !SPRITESHEET2D_HPP_ */
