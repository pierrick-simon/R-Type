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

    inline constexpr float ROOM_HOLE_X = 127.f;
    inline constexpr float ROOM_HOLE_Y = 119.f;
    inline constexpr float BOSS_BACK_X = 147.f;
    inline constexpr float BOSS_WINDOWS_Y = 118.f;

    inline constexpr BossDef BOSS_DEF = {.x = SCREEN_WIDTH -
            (ROOM_HOLE_X + BOSS_BACK_X - LEVEL_END_MARGIN) * LEVEL_SCALE,
        .y = (ROOM_HOLE_Y - BOSS_WINDOWS_Y + 15) * LEVEL_SCALE,
        .enterSpeed = 270.f,
        .closedTime = 2.5f,
        .stageTime = 0.12f,
        .openTime = 3.5f,
        .fireCooldown = 3.f,
        .bulletSpeed = 540.f,
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

    struct BossLineDef {
        int length;
        float interval;
        float turnRate;
        float homingTime;
    };

    inline constexpr BossLineDef BOSS_LINE = {
        .length = 10, .interval = 0.09f, .turnRate = 0.35f, .homingTime = 1.5f};

    struct BossTailDef {
        float anchorX;
        float anchorY;
        int large;
        int medium;
        int small;
        float overlap;
        float tilt;
        float sweep;
        float sweepSpeed;
        float sway;
        float swaySpeed;
        float waveStep;
        float growInterval;
    };

    inline constexpr BossTailDef BOSS_TAIL = {.anchorX = 60.f,
        .anchorY = 186.f,
        .large = 5,
        .medium = 5,
        .small = 4,
        .overlap = 0.75f,
        .tilt = 0.45f,
        .sweep = 0.3f,
        .sweepSpeed = 1.1f,
        .sway = 0.4f,
        .swaySpeed = 3.f,
        .waveStep = 0.65f,
        .growInterval = 0.15f};

    inline constexpr int BOSS_TAIL_SEGMENTS =
        BOSS_TAIL.large + BOSS_TAIL.medium + BOSS_TAIL.small + 1;

    [[nodiscard]] constexpr const SheetInfo &bossTailSheet(int index)
    {
        if (index < BOSS_TAIL.large)
            return BOSS_TAIL_LARGE_SHEET;
        if (index < BOSS_TAIL.large + BOSS_TAIL.medium)
            return BOSS_TAIL_MEDIUM_SHEET;
        if (index < BOSS_TAIL_SEGMENTS - 1)
            return BOSS_TAIL_SMALL_SHEET;
        return BOSS_TAIL_TIP_SHEET;
    }

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
