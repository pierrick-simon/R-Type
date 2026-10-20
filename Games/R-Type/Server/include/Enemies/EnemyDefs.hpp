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

    /**
     * @brief Static description of an enemy kind: adding an enemy is one more
     * line in ENEMY_DEFS (plus one EnemyType value).
     *
     * speed is positive (pixels per second, the enemy always goes left).
     * fireCooldown <= 0 means the enemy does not shoot.
     */
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
                .hp = 3,
                .scoreValue = 300,
                .speed = 180.f,
                .amplitude = 140.f,
                .period = 1.2f,
                .pattern = PatternType::ZigZag,
                .fireCooldown = 1.6f,
                .bulletSpeed = 420.f,
                .bulletDamage = 1},
        }};

    /**
     * @brief Get the description of an enemy kind.
     */
    [[nodiscard]] constexpr const EnemyDef &enemyDef(EnemyType type)
    {
        return ENEMY_DEFS[std::size_t(type)];
    }
} // namespace RType
#endif
