/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** BossDef.hpp
*/

#ifndef BOSSDEF_HPP_
    #define BOSSDEF_HPP_
    #include <utility>
    #include "DimensionsDef.hpp"
    #include "Enemies/EnemyComponents.hpp"
    #include "Enemies/EnemySheets.hpp"
    #include "Position2D.hpp"

namespace RType {

    struct BossDef {
        float x;
        float y;
        float enterSpeed;
        float closedTime;
        float stageTime;
        float openTime;
        float fireCooldown;
        int burst;
        float spread;
        float bulletSpeed;
        int bulletDamage;
        float deathTime;
        float explosionInterval;
        float victoryDelay;
        float hitboxLeft;
        float hitboxTop;
        float hitboxWidth;
        float hitboxHeight;
        float muzzleX;
        float muzzleY;
    };

    inline constexpr float BOSS_WIDTH = float(BOSS_SHEET.cellW) * LEVEL_SCALE;
    inline constexpr float BOSS_HEIGHT = float(BOSS_SHEET.cellH) * LEVEL_SCALE;

    // Entrance of the hole through the back wall, in level pixels from the
    // level's right edge: the boss's back, in BOSS_SHEET pixels, stands on it.
    inline constexpr float ROOM_HOLE_X = 127.f;
    inline constexpr float ROOM_HOLE_Y = 119.f;
    inline constexpr float BOSS_BACK_X = 147.f;
    inline constexpr float BOSS_WINDOWS_Y = 118.f;

    inline constexpr BossDef BOSS_DEF = {.x = SCREEN_WIDTH -
            (ROOM_HOLE_X + BOSS_BACK_X - LEVEL_END_MARGIN) * LEVEL_SCALE,
        .y = (ROOM_HOLE_Y - BOSS_WINDOWS_Y + 30) * LEVEL_SCALE,
        .enterSpeed = 270.f,
        .closedTime = 2.5f,
        .stageTime = 0.12f,
        .openTime = 3.5f,
        .fireCooldown = 0.7f,
        .burst = 3,
        .spread = 0.22f,
        .bulletSpeed = 380.f,
        .bulletDamage = 1,
        .deathTime = 2.5f,
        .explosionInterval = 0.12f,
        .victoryDelay = 3.f,
        .hitboxLeft = 70.f,
        .hitboxTop = 30.f,
        .hitboxWidth = 80.f,
        .hitboxHeight = 160.f,
        .muzzleX = 74.f,
        .muzzleY = 115.f};

    [[nodiscard]] constexpr bool bossExposed(const Boss &boss)
    {
        return boss.state == BossState::Open;
    }

    [[nodiscard]] inline std::pair<float, float> bossMuzzle(
        const Position2D &pos)
    {
        return {pos.x + BOSS_DEF.muzzleX * LEVEL_SCALE,
            pos.y + BOSS_DEF.muzzleY * LEVEL_SCALE};
    }

} // namespace RType

#endif
