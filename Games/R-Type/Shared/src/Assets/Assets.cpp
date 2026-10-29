/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Assets
*/

#include "Assets.hpp"

namespace RType::Asset {

    AssetList makeAssetList()
    {
        return {
            {ENEMY_BULLET, "R-Type/sprites/Enemies/EnemyBullet.gif"},
            {ENEMY_EXPLOSION, "R-Type/sprites/Enemies/ExplosionDead.gif"},
            {ENEMY_ROBOT, "R-Type/sprites/Enemies/RobotEnemy.gif"},
            {ENEMY_SCOUT, "R-Type/sprites/Enemies/scout.gif"},

            {LEVEL1_BACKGROUND, "R-Type/sprites/Levels/Level1_Background.png"},

            {PLAYER_EXPLOSION,
                "R-Type/sprites/PlayerSheets/ExplosionPlayerDead.gif"},
            {PLAYER_BULLET, "R-Type/sprites/PlayerSheets/PlayerBullet.gif"},
            {PLAYER_SHEET, "R-Type/sprites/PlayerSheets/r-typePlayerSheet.gif"},
            {PLAYER_SHEET_1,
                "R-Type/sprites/PlayerSheets/r-typePlayerSheet1.gif"},
            {PLAYER_SHEET_2,
                "R-Type/sprites/PlayerSheets/r-typePlayerSheet2.gif"},
            {PLAYER_SHEET_3,
                "R-Type/sprites/PlayerSheets/r-typePlayerSheet3.gif"},
            {PLAYER_SHEET_4,
                "R-Type/sprites/PlayerSheets/r-typePlayerSheet4.gif"},
            {PLAYER_SHEET_5,
                "R-Type/sprites/PlayerSheets/r-typePlayerSheet5.gif"},
            {FONT,
                "R-Type/sprites/Fonts/r-type.otf"},
        };
    }

} // namespace RType::Asset
