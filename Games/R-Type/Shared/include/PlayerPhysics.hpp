/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PlayerPhysics
*/

#ifndef PlayerPhysics_HPP_
    #define PlayerPhysics_HPP_

namespace RType {
    inline constexpr float PLAYER_MAX_SPEED = 600.f;
    inline constexpr float PLAYER_ACCELERATION = 3000.f;
    inline constexpr float PLAYER_FRICTION =
        PLAYER_ACCELERATION / PLAYER_MAX_SPEED;
} // namespace RType

#endif
