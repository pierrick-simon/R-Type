/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameOver
*/

#ifndef CLIENT_GAMEOVER_HPP_
#define CLIENT_GAMEOVER_HPP_

#include "GameManager.hpp"
#include "Lobby.hpp"

namespace RType::Client {

    inline constexpr float GAME_OVER_TITLE_Y = 380.f;
    // The server lists the scores between the title and the status.
    inline constexpr float GAME_OVER_STATUS_Y = 810.f;
    inline constexpr float GAME_OVER_STATUS_SIZE = 30.f;
    inline constexpr float GAME_OVER_BUTTONS_Y = 850.f;

    struct GameOverItem {};

    struct GameOverButton {
        GameOverChoice choice;
    };

    struct GameOverStatus {};

    void createGameOver(GameManager &gm);
    void destroyGameOver(GameManager &gm);
    void systemGameOver(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_GAMEOVER_HPP_ */
