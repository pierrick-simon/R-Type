/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyDefs.hpp
*/

#ifndef ENEMYDEFS_HPP_
    #define ENEMYDEFS_HPP_
    #include <array>
    #include <cstddef>
    #include "AlienDef.hpp"
    #include "BossDef.hpp"
    #include "DimensionsDef.hpp"
    #include "Enemies/EnemyComponents.hpp"
    #include "Enemies/EnemySheets.hpp"
    #include "ScoutDef.hpp"
    #include "WalkerDef.hpp"

namespace RType {

    struct EnemyDef {
        const char *name;
        const SheetInfo *sheet;
        int hp;
        int scoreValue;
        float speed;
        float amplitude;
        float period;
        PatternType pattern;
        float fireCooldown;
        float bulletSpeed;
        int bulletDamage;
    };

    inline constexpr std::array<EnemyDef, std::size_t(EnemyType::Count)>
        ENEMY_DEFS = {{
            {.name = "Scout",
                .sheet = &SCOUT_SHEET,
                .hp = 1,
                .scoreValue = 100,
                .speed = 260.f * ENEMY_SPEED_FACTOR,
                .amplitude = 120.f,
                .period = 0.5f,
                .pattern = PatternType::ZigZag},
            {.name = "RobotGunner",
                .sheet = &GUNNER_SHEET,
                .hp = 3,
                .scoreValue = 300,
                .speed = 180.f * ENEMY_SPEED_FACTOR,
                .amplitude = 140.f,
                .period = 1.2f,
                .pattern = PatternType::ZigZag,
                .fireCooldown = 1.6f,
                .bulletSpeed = 420.f,
                .bulletDamage = 1},
            {.name = "Walker",
                .sheet = &WALKER_SHEET,
                .hp = 2,
                .scoreValue = 400,
                .speed = 110.f * ENEMY_SPEED_FACTOR,
                .pattern = PatternType::WalkJump,
                .fireCooldown = 1.2f,
                .bulletSpeed = 450.f,
                .bulletDamage = 1},
            {.name = "Alien",
                .sheet = &ALIEN_SHEET,
                .hp = 20,
                .scoreValue = 1000,
                .speed = 260.f * ENEMY_SPEED_FACTOR,
                .pattern = PatternType::AlienBeam},
            {.name = "Boss",
                .sheet = &BOSS_SHEET,
                .hp = 45,
                .scoreValue = 5000,
                .speed = BOSS_DEF.enterSpeed,
                .pattern = PatternType::Boss},
        }};

    [[nodiscard]] constexpr const EnemyDef &enemyDef(EnemyType type)
    {
        return ENEMY_DEFS[std::size_t(type)];
    }
} // namespace RType
#endif
