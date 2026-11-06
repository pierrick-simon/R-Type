/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy.hpp
*/

#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include <cstdint>
#include <optional>
#include "Entity.hpp"

namespace RType {

    enum class EnemyType : std::uint8_t {
        Scout,
        Gunner,
        Walker,
        Alien,
        Boss,
        Count,
    };

    enum class PatternType : std::uint8_t {
        Straight,
        SineWave,
        ZigZag,
        WalkJump,
        AlienBeam,
        Boss,
    };

    struct MovementPattern {
        PatternType type = PatternType::Straight;
        float horizontalVelocity = -300.f;
        float amplitude = 0.f;
        float period = 1.f;
        float phase = 0.f;
        float elapsed = 0.f;
        float zigZagDirection = 1.f;
    };

    enum class WalkerState : std::uint8_t {
        Walk,
        Crouch,
        Jump,
        Land,
    };

    struct Walker {
        WalkerState state = WalkerState::Walk;
        float timer = 0.f;
        float stride = 0.f;
        bool facingLeft = true;
        float jumpVelocityX = 0.f;
        float landY = 0.f;
    };

    enum class AlienState : std::uint8_t {
        Fly,
        Charge,
        Cooldown,
        Descend,
        Landed,
        TakeOff,
        Exit,
    };

    struct Alien {
        AlienState state = AlienState::Fly;
        float timer = 0.f;
        bool facingLeft = true;
        bool grounded = false;
        int shots = 0;
    };

    enum class BossState : std::uint8_t {
        Enter,
        Closed,
        Opening,
        Open,
        Closing,
        Dying,
    };

    struct Boss {
        BossState state = BossState::Enter;
        float timer = 0.f;
        float fireTimer = 0.f;
        int stage = 0;
        int explosions = 0;
    };

    struct Enemy {
        EnemyType type = EnemyType::Scout;
        int scoreValue = 0;
        int contactDamage = 0;
    };

    struct Health {
        int hp = 1;
        int maxHp = 1;
    };

    struct Hitbox {
        float width = 0.f;
        float height = 0.f;
        float offsetX = 0.f;
        float offsetY = 0.f;
    };

    struct Weapon {
        float cooldown = 1.f;
        float timer = 0.f;
        float bulletSpeed = 400.f;
        int damage = 1;
    };

    struct Projectile {
        int damage = 1;
        bool fromPlayer = false;
        bool piercing = false;
        std::optional<Entity> owner = std::nullopt;
    };

    /** @brief Earns the points of the enemies killed by what it owns. */
    struct Scorer {
        int score = 0;
    };

    struct EnemyTarget {};

    struct Invincible {
        float remaining = 0.f;
    };

    struct Lifetime {
        float remaining = 0.f;
    };

    /** @brief Destroys the enemy projectiles that touch it. */
    struct ShotBlocker {};

    /** @brief Hurts the enemies it touches, at most once per cooldown. */
    struct ContactDamage {
        int damage = 1;
        float cooldown = 0.f;
        float timer = 0.f;
        std::optional<Entity> owner = std::nullopt;
    };
} // namespace RType

#endif
