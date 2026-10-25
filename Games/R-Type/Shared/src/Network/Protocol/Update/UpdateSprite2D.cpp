/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateSprite2D
*/

#include "UpdateSprite2D.hpp"
#include "SyncField.hpp"

namespace RType {

    void UpdateSprite2D::systemUpdateSprite2D(
        UpdateSprite2D &update, const Sprite2D &s)
    {
        auto track = [&](auto Sprite2D::*member, Property prop) {
            if (syncField(update.*member, s.*member))
                update._update.push_back(prop);
        };

        track(&Sprite2D::scale, Property::SCALE);
        track(&Sprite2D::rotation, Property::ROTATION);
        track(&Sprite2D::flip, Property::FLIP);
        track(&Sprite2D::color, Property::COLOR);
        track(&Sprite2D::origin, Property::ORIGIN);
    }

    std::optional<SetValue> UpdateSprite2D::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::SCALE:
                return scale;
            case Property::ROTATION:
                return rotation;
            case Property::FLIP:
                return flip;
            case Property::COLOR:
                return int64_t(uint32_t(color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(origin.x), int16_t(origin.y)};
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> UpdateSprite2D::_default = {
        Property::SCALE,
        Property::ROTATION,
        Property::FLIP,
        Property::COLOR,
        Property::ORIGIN,
    };
} // namespace RType
