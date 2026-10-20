/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySheets.hpp
*/

#ifndef ENEMYSHEET_HPP_
#define ENEMYSHEET_HPP_

#include "Assets.hpp"

namespace RType {

    struct SheetInfo {
        AssetId asset;
        int frames;
        int cellW;
        int cellH;
        int frameMs;
        int hitboxW;
        int hitboxH;
    };

    inline constexpr SheetInfo SCOUT_SHEET = {
        Asset::ENEMY_SCOUT, 8, 21, 24, 70, 21, 14};
    inline constexpr SheetInfo GUNNER_SHEET = {
        Asset::ENEMY_ROBOT, 3, 33, 34, 150, 33, 25};

    inline constexpr SheetInfo EXPLOSION_SHEET = {
        Asset::ENEMY_EXPLOSION, 6, 32, 31, 60, 0, 0};

    inline constexpr SheetInfo ENEMY_BULLET_SHEET = {
        Asset::ENEMY_BULLET, 4, 19, 8, 50, 6, 6};
} // namespace RType

#endif
