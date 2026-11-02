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
    };

    inline constexpr SheetInfo SCOUT_SHEET = {
        Asset::ENEMY_SCOUT, 8, 21, 24, 70, 21, 14};
    inline constexpr SheetInfo GUNNER_SHEET = {
        Asset::ENEMY_ROBOT, 3, 33, 34, 150, 33, 25};

    inline constexpr SheetInfo WALKER_SHEET = {
        Asset::ENEMY_WALKER, 12, 33, 33, 120, 26, 30, 2};

    /** @brief Tiles of WALKER_SHEET for one facing direction. */
    struct WalkerFrames {
        std::array<std::size_t, 4> walk;
        std::size_t crouch;
        std::array<std::size_t, 2> jump;
    };

    /** @brief Index 0: facing left, 1: facing right. */
    inline constexpr std::array<WalkerFrames, 2> WALKER_FRAMES = {{
        {{6, 0, 1, 0}, 2, {7, 8}},
        {{11, 5, 4, 5}, 3, {9, 10}},
    }};

    inline constexpr SheetInfo ALIEN_SHEET = {
        Asset::ENEMY_ALIEN, 3, 56, 56, 100, 34, 46};
    inline constexpr SheetInfo ALIEN_BEAM_SHEET = {
        Asset::ALIEN_BEAM, 2, 64, 16, 60, 40, 10};
    inline constexpr SheetInfo ALIEN_CHARGE_SHEET = {
        Asset::ALIEN_CHARGE, 1, 16, 16, 100, 0, 0};

    inline constexpr std::size_t ALIEN_TILE_LANDED = 0;
    inline constexpr std::size_t ALIEN_TILE_LEGS = 1;
    inline constexpr std::size_t ALIEN_TILE_FLY = 2;

    /** @brief Gun tip of each ALIEN_SHEET tile facing left, in pixels. */
    inline constexpr std::array<std::array<float, 2>, 3> ALIEN_MUZZLES = {{
        {0.f, 32.f},
        {0.f, 21.f},
        {1.f, 21.f},
    }};

    inline constexpr SheetInfo BOSS_SHEET = {
        Asset::ENEMY_BOSS, 32, 155, 204, 120, 0, 0, 8, LEVEL_SCALE};
    inline constexpr SheetInfo BOSS_BULLET_SHEET = {
        Asset::BOSS_BULLET, 9, 16, 16, 60, 10, 10};
    inline constexpr SheetInfo BOSS_EXPLOSION_SHEET = {
        Asset::BOSS_EXPLOSION, 8, 64, 64, 80, 0, 0};

    inline constexpr int BOSS_STAGES = 8;
    inline constexpr int BOSS_IDLE_FRAMES = 4;

    inline constexpr SheetInfo EXPLOSION_SHEET = {
        Asset::ENEMY_EXPLOSION, 6, 33, 32, 60, 0, 0};

    inline constexpr SheetInfo ENEMY_BULLET_SHEET = {
        Asset::ENEMY_BULLET, 4, 19, 8, 50, 6, 6};
} // namespace RType

#endif
