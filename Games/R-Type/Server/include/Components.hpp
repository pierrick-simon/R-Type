/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "Config.hpp"
#include "PlayerPhysics.hpp"

namespace RType::Server {

    struct Player {
        float acceleration = PLAYER_ACCELERATION;
        float cooldown = 0.f;
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool dying = false;
        float deathTimer = 0.f;
    };

    struct Bullet {};

    struct Background {};

    struct PlayOnce {};

    struct PlayerExplosion {
        float remaining = 0.f;
    };

} // namespace RType::Server

#endif
