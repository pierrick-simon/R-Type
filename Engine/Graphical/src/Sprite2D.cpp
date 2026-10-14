/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#include "Sprite2D.hpp"

namespace RType {
    void Sprite2D::systemSprite2D(
        Sprite2D &sprite, Texture2D &texture, Position2D &pos)
    {
        auto width = float(texture.width);
        auto height = float(texture.height);
        Rectangle source = {0.0f, 0.0f, width, height};
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
