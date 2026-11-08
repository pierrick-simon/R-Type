/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameState
*/

#ifndef GAMESTATE_HPP_
#define GAMESTATE_HPP_

#include "Clock.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    [[nodiscard]] bool isGameOver(GameManager &gm);

    [[nodiscard]] bool isEndScreen();

    [[nodiscard]] bool isVictoryScreen();

    [[nodiscard]] bool isEndScreenReady();

    void showGameOver(GameManager &gm);

    void showVictory(GameManager &gm);

    void closeEndScreen();

    void resetPlayers(GameManager &gm);

    void restartGame(GameManager &gm);

    void systemGameState(GameManager &gm, const MasterClock &clock);

} // namespace RType::Server

#endif
