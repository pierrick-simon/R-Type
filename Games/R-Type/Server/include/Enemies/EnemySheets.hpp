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

    inline constexpr SheetInfo EXPLOSION_SHEET = {
        Asset::ENEMY_EXPLOSION, 6, 33, 32, 60, 0, 0};

    inline constexpr SheetInfo ENEMY_BULLET_SHEET = {
        Asset::ENEMY_BULLET, 4, 19, 8, 50, 6, 6};
} // namespace RType

#endif
