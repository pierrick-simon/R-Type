/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#include "Sprite2D.hpp"

namespace RType {
    void Sprite2D::systemDrawSprite2D(
        const Sprite2D &sprite, const Texture2D &texture, const Position2D &pos)
    {
        auto width = float(texture.width);
        auto height = float(texture.height);
        Rectangle source = {0.0f, 0.0f, width, height};
        Rectangle dest = {
            pos.x(), pos.y(), width * sprite._scale, height * sprite._scale};
        if (sprite._flip) {
            source.x += width;
            source.width *= -1;
        }
        ::DrawTexturePro(texture,
            source,
            dest,
            {sprite._origin.x, sprite._origin.y},
            sprite._rotation,
            sprite._color);
    }

    std::optional<SetValue> Sprite2D::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::SCALE:
                return _scale;
            case Property::ROTATION:
                return _rotation;
            case Property::FLIP:
                return _flip;
            case Property::COLOR:
                return int64_t(uint32_t(_color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(_origin.x), int16_t(_origin.y)};
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> &Sprite2D::getDefaultUpdate()
    {
        return _default;
    }

    const std::vector<Property> Sprite2D::_default = {
        Property::SCALE,
        Property::ROTATION,
        Property::FLIP,
        Property::COLOR,
        Property::ORIGIN,
    };
}; // namespace RType
