/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#include "Sprite2D.hpp"

namespace RType {
    void Sprite2D::systemSprite2D(Sprite2D &s)
    {
        auto &texture = s.texture.texture;
        auto width = float(texture.width);
        auto height = float(texture.height);
        Rectangle source = {0.0f, 0.0f, width, height};
        Rectangle dest = {s.pos.x, s.pos.y, width * s.scale, height * s.scale};
        ::DrawTexturePro(texture,
            source,
            dest,
            {s.origin.x, s.origin.y},
            s.rotation,
            s.color);
    }
}; // namespace RType
