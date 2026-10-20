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
    };

    struct EnemyTarget {};

    struct Lifetime {
        float remaining = 0.f;
    };
} // namespace RType

#endif
