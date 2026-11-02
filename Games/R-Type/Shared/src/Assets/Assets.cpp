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
            {ALIEN_BEAM, "R-Type/sprites/Enemies/AlienBeam.png"},
            {ALIEN_CHARGE, "R-Type/sprites/Enemies/AlienCharge.png"},
            {ENEMY_ALIEN, "R-Type/sprites/Enemies/Alien.png"},
            {ENEMY_BOSS, "R-Type/sprites/Enemies/Boss.png"},
            {BOSS_BULLET, "R-Type/sprites/Enemies/BossBullet.png"},
            {BOSS_EXPLOSION, "R-Type/sprites/Enemies/BossExplosion.png"},
            {ENEMY_BULLET, "R-Type/sprites/Enemies/EnemyBullet.gif"},
            {ENEMY_EXPLOSION, "R-Type/sprites/Enemies/ExplosionDead.gif"},
            {ENEMY_ROBOT, "R-Type/sprites/Enemies/RobotEnemy.gif"},
            {ENEMY_SCOUT, "R-Type/sprites/Enemies/scout.gif"},
            {ENEMY_WALKER, "R-Type/sprites/r-typesheet7.gif"},

            {LEVEL1_BACKGROUND, "R-Type/sprites/Levels/Level1_Background.png"},
            {LEVEL1_MASK, "R-Type/sprites/Levels/Level1_Background_Masque.png"},

            {PLAYER_EXPLOSION,
                "R-Type/sprites/PlayerSheets/ExplosionPlayerDead.gif"},
            {PLAYER_BULLET, "R-Type/sprites/PlayerSheets/PlayerBullet.gif"},
            {PLAYER_CHARGE, "R-Type/sprites/PlayerSheets/PlayerCharge.png"},
            {PLAYER_CHARGED_SHOT,
                "R-Type/sprites/PlayerSheets/PlayerChargedShot.png"},
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
        };
    }

} // namespace RType::Asset
