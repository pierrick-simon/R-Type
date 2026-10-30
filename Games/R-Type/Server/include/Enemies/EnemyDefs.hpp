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
    #include "EnemyComponents.hpp"
    #include "EnemySheets.hpp"

namespace RType {
    constexpr float SPRITE_SCALE = 3.f;
    constexpr float HITBOX_RATIO = 0.8f;

    constexpr float SCREEN_WIDTH = 1920.f;
    constexpr float SCREEN_HEIGHT = 1080.f;
    constexpr float SPAWN_MARGIN = 80.f;

    struct WalkerDef {
        float stepHeight;
        float crouchTime;
        float turnDelay;
        float jumpApex;
        float gravity;
        float landingDistance;
    };

    /**
     * @brief Walker tuning, in screen pixels. A gap of at most stepHeight
     * between two platforms is walked over; a lower one is jumped down to,
     * landing landingDistance widths past the edge; a higher one makes the
     * walker turn back.
     */
    inline constexpr WalkerDef WALKER_DEF = {.stepHeight = 45.f,
        .crouchTime = 0.25f,
        .turnDelay = 0.3f,
        .jumpApex = 140.f,
        .gravity = 2500.f,
        .landingDistance = 0.8f};

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
                .speed = 260.f,
                .amplitude = 120.f,
                .period = 2.f,
                .pattern = PatternType::SineWave},
            {.name = "RobotGunner",
                .sheet = &GUNNER_SHEET,
                .hp = 1,
                .scoreValue = 300,
                .speed = 180.f,
                .amplitude = 140.f,
                .period = 1.2f,
                .pattern = PatternType::ZigZag,
                .fireCooldown = 1.6f,
                .bulletSpeed = 420.f,
                .bulletDamage = 1},
            {.name = "Walker",
                .sheet = &WALKER_SHEET,
                .hp = 1,
                .scoreValue = 400,
                .speed = 110.f,
                .pattern = PatternType::WalkJump,
                .fireCooldown = 1.2f,
                .bulletSpeed = 450.f,
                .bulletDamage = 1},
        }};

    [[nodiscard]] constexpr const EnemyDef &enemyDef(EnemyType type)
    {
        return ENEMY_DEFS[std::size_t(type)];
    }
} // namespace RType
#endif
