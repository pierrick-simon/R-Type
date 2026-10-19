/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy.hpp
*/

#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include <cstdint>

namespace RType {

    enum class EnemyType : std::uint8_t {
        Scout,
        Gunner,
        Count,
    };

    enum class PatternType : std::uint8_t {
        Straight,
        SineWave,
        ZigZag,
    };

    /**
     * @brief Per-enemy configuration and state for its movement.
     *
     * horizontalVelocity is in pixels per second. For SineWave, amplitude is
     * a vertical displacement in pixels and period is in seconds. For ZigZag,
     * amplitude is a vertical velocity in pixels per second and period is the
     * interval, in seconds, between two direction changes.
     */
    struct MovementPattern {
        PatternType type = PatternType::Straight;
        float horizontalVelocity = -300.f;
        float amplitude = 0.f;
        float period = 1.f;
        float phase = 0.f;
        float elapsed = 0.f;
        float zigZagDirection = 1.f;
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

    /**
     * @brief Axis-aligned collision box, in pixels (already scaled).
     *
     * offsetX / offsetY locate the box relative to Position2D (the top-left
     * corner of the drawn sprite), so the box can be smaller than the sprite.
     */
    struct Hitbox {
        float width = 0.f;
        float height = 0.f;
        float offsetX = 0.f;
        float offsetY = 0.f;
    };

    /** Periodic shot. timer counts up to cooldown (seconds). */
    struct Weapon {
        float cooldown = 1.f;
        float timer = 0.f;
        float bulletSpeed = 400.f;
        int damage = 1;
    };

    /** Anything that hurts: fromPlayer bullets hit enemies, the others hit EnemyTarget. */
    struct Projectile {
        int damage = 1;
        bool fromPlayer = false;
    };

    /** Tag put on the entity enemies aim at and damage (the player). Needs Health, Position2D, Hitbox. */
    struct EnemyTarget {};

    /** The entity is destroyed once remaining (seconds) reaches 0. */
    struct Lifetime {
        float remaining = 0.f;
    };
} // namespace RType

#endif /* !ENEMY_HPP_ */
