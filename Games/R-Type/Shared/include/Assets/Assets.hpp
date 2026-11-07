/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Assets
*/

#ifndef ASSETS_HPP_
#define ASSETS_HPP_

#include <string>
#include <unordered_map>

namespace RType {

    using AssetId = int;

    using AssetList = std::unordered_map<AssetId, std::string>;

} // namespace RType

namespace RType::Asset {

    inline constexpr AssetId FIRST_ID = 100;

    inline constexpr AssetId ENEMY_BULLET = 100;
    inline constexpr AssetId ENEMY_EXPLOSION = 101;
    inline constexpr AssetId ENEMY_ROBOT = 102;
    inline constexpr AssetId ENEMY_SCOUT = 103;
    inline constexpr AssetId LEVEL1_BACKGROUND = 104;
    inline constexpr AssetId PLAYER_EXPLOSION = 105;
    inline constexpr AssetId PLAYER_BULLET = 106;
    inline constexpr AssetId PLAYER_SHEET = 107;
    inline constexpr AssetId PLAYER_SHEET_1 = 108;
    inline constexpr AssetId PLAYER_SHEET_2 = 109;
    inline constexpr AssetId PLAYER_SHEET_3 = 110;
    inline constexpr AssetId PLAYER_SHEET_4 = 111;
    inline constexpr AssetId PLAYER_SHEET_5 = 112;
    inline constexpr AssetId ENEMY_WALKER = 113;
    inline constexpr AssetId LEVEL1_MASK = 114;
    inline constexpr AssetId ENEMY_ALIEN = 115;
    inline constexpr AssetId ALIEN_BEAM = 116;
    inline constexpr AssetId ALIEN_CHARGE = 117;
    inline constexpr AssetId PLAYER_CHARGE = 118;
    inline constexpr AssetId PLAYER_CHARGED_SHOT = 119;
    inline constexpr AssetId ENEMY_BOSS = 120;
    inline constexpr AssetId BOSS_BULLET = 121;
    inline constexpr AssetId BOSS_EXPLOSION = 122;
    inline constexpr AssetId POWER_UP = 123;
    inline constexpr AssetId FORCE = 124;
    inline constexpr AssetId REFLECT_LASER = 125;
    inline constexpr AssetId HELIX_LASER = 126;
    inline constexpr AssetId RING_LASER = 127;
    inline constexpr AssetId LASER_MUZZLE = 128;

    inline constexpr AssetId EXPLOSION_SOUND = 1000;
    inline constexpr AssetId GUNSHOT_SOUND = 1001;

    inline constexpr AssetId FIRST_LEVEL_MUSIC = 2001;

    inline constexpr AssetId FONT = 200;

    inline constexpr AssetId MENU_BACKGROUND = 300;
    inline constexpr AssetId MENU_BUTTON = 301;
    inline constexpr AssetId MENU_PLAYER_BACK = 302;
    inline constexpr AssetId MENU_BACK = 303;
    inline constexpr AssetId MENU_BACK_SIDE = 304;
    inline constexpr AssetId MENU_LIST_FRAME = 305;
    inline constexpr AssetId MENU_VOLUME_DOWN = 306;
    inline constexpr AssetId MENU_VOLUME_UP = 307;

    [[nodiscard]] AssetList makeAssetList();

} // namespace RType::Asset

#endif
