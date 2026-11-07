/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AlienDef.hpp
*/

#ifndef ALIENDEF_HPP_
    #define ALIENDEF_HPP_
    #include <cstddef>
    #include <utility>
    #include "DimensionsDef.hpp"
    #include "Enemies/EnemyComponents.hpp"
    #include "Enemies/EnemySheets.hpp"
    #include "Position2D.hpp"

namespace RType {

    struct AlienDef {
        float hoverX;
        float minY;
        float alignTolerance;
        float chargeTime;
        float cooldown;
        float beamSpeed;
        int beamDamage;
        int shotsBeforeLanding;
        int maxShots;
        float landSpeed;
        float landedTime;
        float takeOffTime;
    };

    inline constexpr AlienDef ALIEN_DEF = {
        .hoverX = 1450.f,
        .minY = 100.f,
        .alignTolerance = 12.f,
        .chargeTime = 0.8f,
        .cooldown = 0.6f,
        .beamSpeed = 1100.f,
        .beamDamage = 1,
        .shotsBeforeLanding = 2,
        .maxShots = 10,
        .landSpeed = 320.f,
        .landedTime = 2.f,
        .takeOffTime = 0.4f};

    [[nodiscard]] constexpr std::size_t alienTile(const Alien &alien)
    {
        if (alien.state == AlienState::Descend ||
            alien.state == AlienState::TakeOff)
            return ALIEN_TILE_LEGS;
        return alien.grounded ? ALIEN_TILE_LANDED : ALIEN_TILE_FLY;
    }

    /** @brief Screen position of the alien's gun tip. */
    [[nodiscard]] inline std::pair<float, float> alienMuzzle(
        const Alien &alien, const Position2D &pos)
    {
        const auto &muzzle = ALIEN_MUZZLES[alienTile(alien)];
        const float x =
            alien.facingLeft ? muzzle[0] : float(ALIEN_SHEET.cellW) - muzzle[0];

        return {pos.x + x * SPRITE_SCALE, pos.y + muzzle[1] * SPRITE_SCALE};
    }

} // namespace RType

#endif
