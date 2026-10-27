/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateSpriteSheet2D
*/

#include "UpdateSpriteSheet2D.hpp"
#include "SyncField.hpp"

namespace RType {

    void UpdateSpriteSheet2D::systemUpdateSpriteSheet2D(
        UpdateSpriteSheet2D &update, const SpriteSheet2D &ss)
    {
        auto track = [&](auto SpriteSheet2D::*member, Property prop) {
            if (syncField(update.*member, ss.*member))
                update._update.push_back(prop);
        };

        track(&SpriteSheet2D::scale, Property::SCALE);
        track(&SpriteSheet2D::rotation, Property::ROTATION);
        track(&SpriteSheet2D::flip, Property::FLIP);
        track(&SpriteSheet2D::color, Property::COLOR);
        track(&SpriteSheet2D::origin, Property::ORIGIN);
        track(&SpriteSheet2D::currentTile, Property::FRAME);
        track(&SpriteSheet2D::frameDuration, Property::FRAME_DURATION);
        track(&SpriteSheet2D::recNb, Property::REC_NB);
    }

    std::optional<SetValue> UpdateSpriteSheet2D::getProperty(
        Property prop) const
    {
        switch (prop) {
            case Property::SCALE:
                return scale;
            case Property::ROTATION:
                return rotation;
            case Property::FLIP:
                return int64_t(flip);
            case Property::COLOR:
                return int64_t(uint32_t(color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(origin.x), int16_t(origin.y)};
            case Property::FRAME:
                return int64_t(currentTile);
            case Property::FRAME_DURATION:
                return int64_t(frameDuration.count());
            case Property::REC_NB:
                return Point16 {int16_t(recNb.height), int16_t(recNb.width)};
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> UpdateSpriteSheet2D::_default = {
        Property::SCALE,
        Property::ROTATION,
        Property::FLIP,
        Property::COLOR,
        Property::ORIGIN,
        Property::FRAME,
        Property::FRAME_DURATION,
        Property::REC_NB,
    };
} // namespace RType
