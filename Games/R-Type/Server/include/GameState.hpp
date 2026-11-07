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

    /**
     * @return true once every player is out of lives and done exploding,
     * false if there is no player at all.
     */
    [[nodiscard]] bool isGameOver(GameManager &gm);

    /** @return true while the game over or victory screen is shown. */
    [[nodiscard]] bool isEndScreen();

    /** @return true while the victory screen is shown. */
    [[nodiscard]] bool isVictoryScreen();

    /** @return true once the level is cleared for the scores, after the
     * victory's fade to black. */
    [[nodiscard]] bool isEndScreenReady();

    /**
     * @brief Clears the level and waits for the players to restart or for the
     * host to go back to the lobby, the scores stay.
     */
    void showGameOver(GameManager &gm);

    /** @brief Same as showGameOver once the boss is beaten, after the fade
     * to black of the clients: the surviving ships are hidden then. */
    void showVictory(GameManager &gm);

    /** @brief Leaves the end screen without restarting. */
    void closeEndScreen();

    /** @brief Full lives, a fresh ship and no points for every player. */
    void resetPlayers(GameManager &gm);

    /**
     * @brief Gives every player back their lives and restarts the level from
     * its first screen.
     */
    void restartGame(GameManager &gm);

    /**
     * @brief Shows the game over screen when nobody has lives left, the
     * victory screen once the boss is beaten.
     */
    void systemGameState(GameManager &gm, const MasterClock &clock);

} // namespace RType::Server

#endif
