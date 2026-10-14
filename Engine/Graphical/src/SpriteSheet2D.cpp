/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#include "SpriteSheet2D.hpp"
#include <cmath>

namespace RType {
    void SpriteSheet2D::systemSpriteSheet2D(SpriteSheet2D &s)
    {
        s.sleep += s.clock.getElapsed();
        if (s.sleep > s.frameDuration) {
            s.currentTile++;
            s.sleep = std::chrono::milliseconds(0);
        }
        auto width = float(s.texture.width) / float(s.recNb.width);
        auto height = float(s.texture.height) / float(s.recNb.height);
        std::size_t nbTile = s.recNb.width * s.recNb.height;
        s.currentTile %= nbTile;
        std::size_t row = s.currentTile / s.recNb.width;
        auto left = float(s.currentTile % s.recNb.width) * width;
        auto top = float(row) * height;
        Rectangle source = {left, top, width, height};
        Rectangle dest = {s.pos.x, s.pos.y, width * s.scale, height * s.scale};
        if (s.flip) {
            source.x += width;
            source.width *= -1;
        }
        ::DrawTexturePro(s.texture,
            source,
            dest,
            {s.origin.x, s.origin.y},
            s.rotation,
            s.color);
    }
}; // namespace RType
