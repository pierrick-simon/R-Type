/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2D
*/

#include "Text2D.hpp"

namespace RType {
    void Text2D::systemDrawText2D(
        const Text2D &text, const Font &font, const Position2D &pos)
    {
        ::DrawTextPro(font,
            text._str.c_str(),
            {pos.x(), pos.y()},
            {text._origin.x, text._origin.y},
            text._rotation,
            text._fontSize,
            text._spacing,
            text._color);
    }

    std::optional<SetValue> Text2D::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::FONT_SIZE:
                return _fontSize;
            case Property::ROTATION:
                return _rotation;
            case Property::SPACING:
                return _spacing;
            case Property::COLOR:
                return int64_t(uint32_t(_color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(_origin.x), int16_t(_origin.y)};
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> Text2D::_default = {
        Property::FONT_SIZE,
        Property::ROTATION,
        Property::SPACING,
        Property::COLOR,
        Property::ORIGIN,
    };
} // namespace RType
