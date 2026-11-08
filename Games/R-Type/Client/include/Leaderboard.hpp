/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Leaderboard
*/

#ifndef LEADERBOARD_HPP_
#define LEADERBOARD_HPP_

#include <chrono>
#include <cstddef>
#include "AccountProtocol.hpp"
#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr float LEADERBOARD_TITLE_Y = 110.f;
    inline constexpr float LEADERBOARD_FIRST_ROW_Y = 220.f;
    inline constexpr float LEADERBOARD_YOU_LABEL_Y = 710.f;
    inline constexpr float LEADERBOARD_YOU_ROW_Y = 750.f;
    inline constexpr float LEADERBOARD_TEXT_SIZE = 44.f;
    inline constexpr std::chrono::milliseconds LEADERBOARD_RESEND_RATE {500};

    /** @brief Row of the player looking at the page (after the top 5). */
    inline constexpr std::size_t LEADERBOARD_YOU_ROW = LEADERBOARD_SIZE;

    /** @brief Tag of every entity of the leaderboard page. */
    struct LeaderboardItem {};

    /** @brief Background (MENU_PLAYER_BACK) of one row. */
    struct LeaderboardRowBack {
        std::size_t row;
    };

    /** @brief "PLACE - PSEUDO - SCORE" text of one row. */
    struct LeaderboardRowText {
        std::size_t row;
    };

    /** @brief Request state of the page, on the master connection. */
    struct LeaderboardClient {
        bool shown = false;
        bool answered = false;
        std::chrono::steady_clock::time_point lastRequest {};
    };

    void openLeaderboard(GameManager &gm);
    void closeLeaderboard(GameManager &gm);

    /**
     * @brief Asks the master for the leaderboard while the page is open
     *        (every 500 ms until it answers), then fills the rows.
     */
    void systemLeaderboard(GameManager &gm);

} // namespace RType::Client

#endif /* !LEADERBOARD_HPP_ */
