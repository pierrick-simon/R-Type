/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameState
*/

#include "GameState.hpp"

#include <algorithm>
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
        enum class EndScreen { None, GameOver, Victory };

        thread_local EndScreen endScreen = EndScreen::None;

        bool levelRunning(GameManager &gm)
        {
            return std::ranges::any_of(gm.view<EnemyDirector>().getVector(),
                [](const auto &entry) { return !std::get<1>(entry).paused; });
        }

        void showEndScreen(GameManager &gm, EndScreen screen)
        {
            std::vector<Entity> backgrounds;

            endScreen = screen;
            clearEnemies(gm);
            for (auto [_, director] : gm.view<EnemyDirector>().getVector())
                director.paused = true;
            // The clients draw the menu background in its place.
            for (auto [background, tag] : gm.view<Background>().getVector())
                backgrounds.push_back(background);
            for (const Entity background : backgrounds)
                destroyNetworked(gm, background);
        }
    } // namespace

    bool isEndScreen()
    {
        return endScreen != EndScreen::None;
    }

    bool isVictoryScreen()
    {
        return endScreen == EndScreen::Victory;
    }

    void showGameOver(GameManager &gm)
    {
        showEndScreen(gm, EndScreen::GameOver);
    }

    void showVictory(GameManager &gm)
    {
        showEndScreen(gm, EndScreen::Victory);
        // The survivors leave the screen to the scores, until the restart.
        for (auto [entity, player, sheet] :
            gm.view<Player, SpriteSheet2D>().getVector()) {
            player.dying = true;
            player.deathTimer = 0.f;
            player.charging = false;
            sheet.color.a = 0;
            gm.getComponent<Velocity2D>(entity) = {};
            if (gm.hasComponent<Invincible>(entity))
                gm.removeComponent<Invincible>(entity);
        }
    }

    void closeEndScreen()
    {
        endScreen = EndScreen::None;
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
        closeEndScreen();
        resetPlayers(gm);
        resetEnemies(gm);
        if (gm.view<Background>().getVector().empty())
            createBackground(gm);
        else
            resetBackground(gm);
    }

    void systemGameState(GameManager &gm)
    {
        if (isEndScreen() || !levelRunning(gm))
            return;
        if (isGameOver(gm))
            showGameOver(gm);
        else if (levelCleared(gm))
            showVictory(gm);
    }

} // namespace RType::Server
