/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
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
        int lives = PLAYER_LIVES;
        std::size_t slot = 0;
    };

    struct LifeIcon {
        Entity owner {0};
        int index = 0;
    };

    /** @brief Bottom HUD score, of the team when it has no player. */
    struct ScoreLabel {
        std::optional<Entity> player;
    };

    struct Bullet {};

    struct ChargedShot {
        std::size_t level = 0;
    };

    struct ChargeEffect {
        Entity owner;
    };

    struct Background {};

    struct PlayOnce {};

    struct PlayerExplosion {
        float remaining = 0.f;
    };

    enum class LaserType : std::uint8_t {
        Reflect,
        Helix,
        Ring,
        Count,
    };

    struct PowerUp {
        LaserType laser = LaserType::Reflect;
    };

    struct Force {
        Entity owner {0};
        int level = 1;
        LaserType laser = LaserType::Reflect;
    };

    struct ReflectShot {
        std::uint32_t beam = 0;
        int pieces = 1;
        float tipTimer = 0.f;
        int bounces = 0;
    };

    /** @brief Enemy shot that left the walls it was fired from. */
    struct ClearOfTerrain {};

    struct RingShot {};

} // namespace RType::Server

#endif
