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
    inline constexpr AssetId FONT = 200;

    [[nodiscard]] AssetList makeAssetList();

} // namespace RType::Asset

#endif
