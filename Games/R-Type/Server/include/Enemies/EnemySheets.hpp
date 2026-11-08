/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySheets.hpp
*/

#ifndef ENEMYSHEET_HPP_
#define ENEMYSHEET_HPP_

#include <array>
#include <cstddef>
#include "Assets.hpp"
#include "Enemies/Defs/DimensionsDef.hpp"

namespace RType {

    struct SheetInfo {
        AssetId asset = 0;
        int frames = 0;
        int cellW = 0;
        int cellH = 0;
        int frameMs = 0;
        int hitboxW = 0;
        int hitboxH = 0;
        int rows = 1;
        float scale = SPRITE_SCALE;
        bool flashFrames = false;
    };

    inline constexpr SheetInfo SCOUT_SHEET = {
        Asset::ENEMY_SCOUT, 8, 21, 24, 70, 21, 14, 1, SPRITE_SCALE, true};
    inline constexpr SheetInfo GUNNER_SHEET = {
        Asset::ENEMY_ROBOT, 3, 33, 34, 150, 33, 25, 1, SPRITE_SCALE, true};

    inline constexpr SheetInfo WALKER_SHEET = {
        Asset::ENEMY_WALKER, 12, 33, 33, 120, 26, 30, 2, SPRITE_SCALE, true};

    struct WalkerFrames {
        std::array<std::size_t, 4> walk;
        std::size_t crouch;
        std::array<std::size_t, 2> jump;
    };

    inline constexpr std::array<WalkerFrames, 2> WALKER_FRAMES = {{
        {{6, 0, 1, 0}, 2, {7, 8}},
        {{11, 5, 4, 5}, 3, {9, 10}},
    }};

    inline constexpr SheetInfo ALIEN_SHEET = {
        Asset::ENEMY_ALIEN, 3, 56, 56, 100, 34, 46, 1, SPRITE_SCALE, true};
    inline constexpr SheetInfo ALIEN_BEAM_SHEET = {
        Asset::ALIEN_BEAM, 2, 64, 16, 60, 40, 10};
    inline constexpr SheetInfo ALIEN_CHARGE_SHEET = {
        Asset::ALIEN_CHARGE, 1, 16, 16, 100, 0, 0};

    inline constexpr std::size_t ALIEN_TILE_LANDED = 0;
    inline constexpr std::size_t ALIEN_TILE_LEGS = 1;
    inline constexpr std::size_t ALIEN_TILE_FLY = 2;

    inline constexpr std::array<std::array<float, 2>, 3> ALIEN_MUZZLES = {{
        {0.f, 32.f},
        {0.f, 21.f},
        {1.f, 21.f},
    }};

    inline constexpr SheetInfo BOSS_SHEET = {
        Asset::ENEMY_BOSS, 64, 155, 204, 120, 0, 0, 16, LEVEL_SCALE};
    inline constexpr SheetInfo BOSS_BULLET_SHEET = {
        Asset::BOSS_BULLET, 4, 22, 20, 70, 14, 14};
    inline constexpr SheetInfo BOSS_TAIL_LARGE_SHEET = {
        Asset::BOSS_TAIL_LARGE, 1, 16, 15, 100, 12, 11, 1, LEVEL_SCALE};
    inline constexpr SheetInfo BOSS_TAIL_MEDIUM_SHEET = {
        Asset::BOSS_TAIL_MEDIUM, 1, 14, 13, 100, 10, 9, 1, LEVEL_SCALE};
    inline constexpr SheetInfo BOSS_TAIL_SMALL_SHEET = {
        Asset::BOSS_TAIL_SMALL, 1, 12, 11, 100, 8, 7, 1, LEVEL_SCALE};
    inline constexpr SheetInfo BOSS_TAIL_TIP_SHEET = {
        Asset::BOSS_TAIL_TIP, 9, 16, 16, 100, 12, 12, 1, LEVEL_SCALE};
    inline constexpr int BOSS_TAIL_TIP_DOWN = 0;
    inline constexpr int BOSS_TAIL_TIP_UP = 8;
    inline constexpr SheetInfo BOSS_EXPLOSION_SHEET = {
        Asset::BOSS_EXPLOSION, 8, 64, 64, 80, 0, 0};

    inline constexpr int BOSS_STAGES = 8;
    inline constexpr int BOSS_IDLE_FRAMES = 4;
    inline constexpr std::size_t BOSS_FLASH_OFFSET =
        std::size_t(BOSS_STAGES) * std::size_t(BOSS_IDLE_FRAMES);
    inline constexpr float ENEMY_HURT_FLASH = 0.15f;
    inline constexpr float ENEMY_HURT_BLINK = 0.05f;

    inline constexpr SheetInfo EXPLOSION_SHEET = {
        Asset::ENEMY_EXPLOSION, 6, 32, 31, 67, 0, 0};
    inline constexpr SheetInfo ENEMY_BULLET_SHEET = {
        Asset::ENEMY_BULLET, 4, 19, 8, 50, 6, 6};
} // namespace RType

#endif
