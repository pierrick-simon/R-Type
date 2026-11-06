/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameOver
*/

#ifndef CLIENT_GAMEOVER_HPP_
#define CLIENT_GAMEOVER_HPP_

#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr float GAME_OVER_TITLE_Y = 390.f;
    inline constexpr float GAME_OVER_STATUS_Y = 590.f;
    inline constexpr float GAME_OVER_STATUS_SIZE = 30.f;
    inline constexpr float GAME_OVER_BUTTONS_Y = 640.f;

    // The server draws the team score between the title and the status.

    struct GameOverItem {};

    struct GameOverButton {};

    struct GameOverStatus {};

    void createGameOver(GameManager &gm);
    void destroyGameOver(GameManager &gm);
    void systemGameOver(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_GAMEOVER_HPP_ */
