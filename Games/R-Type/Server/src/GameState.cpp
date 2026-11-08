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
#include "LobbyProtocol.hpp"
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
        thread_local float fadeLeft = 0.f;

        bool levelRunning(GameManager &gm)
        {
            return std::ranges::any_of(gm.view<EnemyDirector>().getVector(),
                [](const auto &entry) { return !std::get<1>(entry).paused; });
        }

        void pauseLevel(GameManager &gm)
        {
            for (auto [_, director] : gm.view<EnemyDirector>().getVector())
                director.paused = true;
        }

        void clearLevel(GameManager &gm)
        {
            std::vector<Entity> backgrounds;

            clearEnemies(gm);
            pauseLevel(gm);
            // The clients draw the menu background in its place.
            for (auto [background, tag] : gm.view<Background>().getVector())
                backgrounds.push_back(background);
            for (const Entity background : backgrounds)
                destroyNetworked(gm, background);
        }

        /** @brief The survivors leave the screen to the scores. */
        void hideSurvivors(GameManager &gm)
        {
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
    } // namespace

    bool isEndScreen()
    {
        return endScreen != EndScreen::None;
    }

    bool isVictoryScreen()
    {
        return endScreen == EndScreen::Victory;
    }

    bool isEndScreenReady()
    {
        return isEndScreen() && fadeLeft <= 0.f;
    }

    void showGameOver(GameManager &gm)
    {
        endScreen = EndScreen::GameOver;
        fadeLeft = 0.f;
        clearLevel(gm);
    }

    void showVictory(GameManager &gm)
    {
        endScreen = EndScreen::Victory;
        fadeLeft = VICTORY_FADE_SECONDS;
        pauseLevel(gm);
    }

    void closeEndScreen()
    {
        endScreen = EndScreen::None;
        fadeLeft = 0.f;
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

    void systemGameState(GameManager &gm, const MasterClock &clock)
    {
        // The level stays on screen while the clients fade it to black.
        if (fadeLeft > 0.f) {
            fadeLeft -= elapsedSeconds(clock);
            if (fadeLeft <= 0.f) {
                clearLevel(gm);
                hideSurvivors(gm);
            }
            return;
        }
        if (isEndScreen() || !levelRunning(gm))
            return;
        if (isGameOver(gm))
            showGameOver(gm);
        else if (levelCleared(gm))
            showVictory(gm);
    }

} // namespace RType::Server
