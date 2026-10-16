/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#include "SpriteSheet2D.hpp"
#include <cmath>

namespace RType {
    void SpriteSheet2D::systemSpriteSheet2D(SpriteSheet2D &sprite,
        const Texture2D &texture, const Position2D &pos, const Clock &clock)
    {
        sprite.sleep += clock.getElapsed();
        if (sprite.sleep > sprite.frameDuration && sprite.animation) {
            sprite.currentTile++;
            sprite.sleep = std::chrono::milliseconds(0);
        }
        auto width = float(texture.width) / float(sprite.recNb.width);
        auto height = float(texture.height) / float(sprite.recNb.height);
        std::size_t nbTile = sprite.recNb.width * sprite.recNb.height;
        sprite.currentTile %= nbTile;
        std::size_t row = sprite.currentTile / sprite.recNb.width;
        auto left = float(sprite.currentTile % sprite.recNb.width) * width;
        auto top = float(row) * height;
        Rectangle source = {left, top, width, height};
        Rectangle dest = {
            pos.x, pos.y, width * sprite.scale, height * sprite.scale};
        if (sprite.flip) {
            source.x += width;
            source.width *= -1;
        }
        ::DrawTexturePro(texture,
            source,
            dest,
            {sprite.origin.x, sprite.origin.y},
            sprite.rotation,
            sprite.color);
    }
}; // namespace RType
