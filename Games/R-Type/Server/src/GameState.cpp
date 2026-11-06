/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameState
*/

#include "GameState.hpp"

#include <vector>

#include "Common/Background.hpp"
#include "Components.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Player/Destroy.hpp"
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

    namespace {
        thread_local bool gameOver = false;

        bool levelRunning(GameManager &gm)
        {
            for (auto [_, director] : gm.view<EnemyDirector>().getVector())
                if (!director.paused)
                    return true;
            return false;
        }
    } // namespace

    bool isGameOverScreen()
    {
        return gameOver;
    }

    void showGameOver(GameManager &gm)
    {
        std::vector<Entity> backgrounds;

        gameOver = true;
        clearEnemies(gm);
        for (auto [_, director] : gm.view<EnemyDirector>().getVector())
            director.paused = true;
        // The clients draw the menu background in its place.
        for (auto [background, tag] : gm.view<Background>().getVector())
            backgrounds.push_back(background);
        for (const Entity background : backgrounds)
            destroyNetworked(gm, background);
    }

    void closeGameOver()
    {
        gameOver = false;
    }

    void resetPlayers(GameManager &gm)
    {
        for (auto [entity, player] : gm.view<Player>().getVector()) {
            player.lives = PLAYER_LIVES;
            if (gm.hasComponent<Scorer>(entity))
                gm.getComponent<Scorer>(entity).score = 0;
            respawnPlayer(gm, entity);
        }
    }

    void restartGame(GameManager &gm)
    {
        closeGameOver();
        resetPlayers(gm);
        resetEnemies(gm);
        if (gm.view<Background>().getVector().empty())
            createBackground(gm);
        else
            resetBackground(gm);
    }

    void systemGameState(GameManager &gm)
    {
        if (gameOver || !levelRunning(gm))
            return;
        if (isGameOver(gm))
            showGameOver(gm);
        else if (levelCleared(gm))
            restartGame(gm);
    }

} // namespace RType::Server
