/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DimensionsDef.hpp
*/

#ifndef DIMENSIONSDEF_HPP_
    #define DIMENSIONSDEF_HPP_

namespace RType {

    constexpr float SPRITE_SCALE = 3.f;
    constexpr float LEVEL_SCALE = 4.5f;
    constexpr float HITBOX_RATIO = 0.8f;

    constexpr float SCREEN_WIDTH = 1920.f;
    constexpr float SCREEN_HEIGHT = 1080.f;
    constexpr float SPAWN_MARGIN = 80.f;
    // Level pixels left off screen, past the right edge, once the scroll
    // stops: more room for the players in front of the boss.
    constexpr float LEVEL_END_MARGIN = 40.f;

} // namespace RType

#endif
