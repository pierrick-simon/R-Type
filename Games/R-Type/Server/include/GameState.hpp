/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameState
*/

#ifndef GAMESTATE_HPP_
#define GAMESTATE_HPP_

#include "GameManager.hpp"

namespace RType::Server {

    /**
     * @return true once every player is out of lives and done exploding,
     * false if there is no player at all.
     */
    [[nodiscard]] bool isGameOver(GameManager &gm);

    /**
     * @brief Gives every player back their lives and restarts the level from
     * its first screen.
     */
    void restartGame(GameManager &gm);

    /** @brief Restarts the game on a game over or once the boss is beaten. */
    void systemGameState(GameManager &gm);

} // namespace RType::Server

#endif
