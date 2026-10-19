/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "Config.hpp"

namespace RType::Server {

    struct Player {
        float acceleration = PLAYER_ACCELERATION;
        float cooldown = 0.f;
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    };

    struct Bullet {
    };

    struct Background {
    };

}

#endif
