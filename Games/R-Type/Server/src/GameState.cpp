/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameState
*/

#include "GameState.hpp"

#include "Common/Background.hpp"
#include "Components.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Player/Player.hpp"

namespace RType::Server {

    bool isGameOver(GameManager &gm)
    {
        bool anyPlayer = false;

        for (auto [_, player] : gm.view<Player>().getVector()) {
            anyPlayer = true;
            if (player.lives > 0 || player.deathTimer > 0.f)
                return false;
        }
        return anyPlayer;
    }

    void restartGame(GameManager &gm)
    {
        for (auto [entity, player] : gm.view<Player>().getVector()) {
            player.lives = PLAYER_LIVES;
            respawnPlayer(gm, entity);
        }
        resetEnemies(gm);
        resetBackground(gm);
    }

    void systemGameState(GameManager &gm)
    {
        if (isGameOver(gm) || levelCleared(gm))
            restartGame(gm);
    }

} // namespace RType::Server
