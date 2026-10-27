/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateText2D
*/

#include "UpdateText2D.hpp"
#include "SyncField.hpp"

namespace RType {

    void UpdateText2D::systemUpdateText2D(
        UpdateText2D &update, const Text2D &text)
    {
        auto track = [&](auto Text2D::*member, Property prop) {
            if (syncField(update.*member, text.*member))
                update._update.push_back(prop);
        };

        update._updateText = syncField(update.str, text.str);

        track(&Text2D::fontSize, Property::FONT_SIZE);
        track(&Text2D::rotation, Property::ROTATION);
        track(&Text2D::spacing, Property::SPACING);
        track(&Text2D::color, Property::COLOR);
        track(&Text2D::origin, Property::ORIGIN);
    }

    std::optional<SetValue> UpdateText2D::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::FONT_SIZE:
                return fontSize;
            case Property::ROTATION:
                return rotation;
            case Property::SPACING:
                return spacing;
            case Property::COLOR:
                return int64_t(uint32_t(color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(origin.x), int16_t(origin.y)};
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> UpdateText2D::_default = {
        Property::FONT_SIZE,
        Property::ROTATION,
        Property::SPACING,
        Property::COLOR,
        Property::ORIGIN,
    };
} // namespace RType
