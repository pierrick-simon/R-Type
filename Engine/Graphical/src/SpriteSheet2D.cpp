/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#include "SpriteSheet2D.hpp"
#include <cmath>

namespace RType {

    void SpriteSheet2D::systemDrawSpriteSheet2D(
        SpriteSheet2D &sprite, const Texture2D &texture, const Position2D &pos)
    {
        auto width = float(texture.width) / float(sprite._recNb.width);
        auto height = float(texture.height) / float(sprite._recNb.height);
        std::size_t nbTile = sprite._recNb.width * sprite._recNb.height;
        sprite._currentTile %= nbTile;
        std::size_t row = sprite._currentTile / sprite._recNb.width;
        auto left = float(sprite._currentTile % sprite._recNb.width) * width;
        auto top = float(row) * height;
        Rectangle source = {left, top, width, height};
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

    void SpriteSheet2D::systemAnimateSpriteSheet2D(
        SpriteSheet2D &sprite, const Clock &clock)
    {
        sprite._sleep += clock.getElapsed();
        if (sprite._sleep > sprite._frameDuration && sprite._animation) {
            sprite._currentTile++;
            sprite._sleep = std::chrono::milliseconds(0);
        }
    }

    void SpriteSheet2D::systemMasterAnimateSpriteSheet2D(
        GameManager &gm, const MasterClock &clock)
    {

        for (auto [_, sprite] : gm.view<SpriteSheet2D>().getVector())
            systemAnimateSpriteSheet2D(sprite, clock);
    }

    std::optional<SetValue> SpriteSheet2D::getProperty(Property prop) const
    {
        switch (prop) {
            case Property::SCALE:
                return _scale;
            case Property::ROTATION:
                return _rotation;
            case Property::FLIP:
                return int64_t(_flip);
            case Property::COLOR:
                return int64_t(uint32_t(_color.ToInt()));
            case Property::ORIGIN:
                return Point16 {int16_t(_origin.x), int16_t(_origin.y)};
            case Property::FRAME:
                return int64_t(_currentTile);
            case Property::FRAME_DURATION:
                return int64_t(_frameDuration.count());
            default:
                return std::nullopt;
        }
    }

    const std::vector<Property> &SpriteSheet2D::getDefaultUpdate()
    {
        return _default;
    }

    const std::vector<Property> SpriteSheet2D::_default = {
        Property::SCALE,
        Property::ROTATION,
        Property::FLIP,
        Property::COLOR,
        Property::ORIGIN,
        Property::FRAME,
        Property::FRAME_DURATION,
    };
}; // namespace RType
