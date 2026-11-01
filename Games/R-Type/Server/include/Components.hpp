/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include <cstddef>
#include "Config.hpp"
#include "Entity.hpp"
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
        bool charging = false;
        float charge = 0.f;
    };

    struct Bullet {};

    /** @brief Beam fired on release of a charge, piercing what it kills. */
    struct ChargedShot {
        std::size_t level = 0;
    };

    /** @brief Glow at the nose of a player charging a shot. */
    struct ChargeEffect {
        Entity owner;
    };

    struct Background {};

    struct PlayOnce {};

    struct PlayerExplosion {
        float remaining = 0.f;
    };

} // namespace RType::Server

#endif
