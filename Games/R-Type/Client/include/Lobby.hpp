/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lobby
*/

#ifndef CLIENT_LOBBY_HPP_
#define CLIENT_LOBBY_HPP_

#include <chrono>
#include <cstddef>

#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr std::chrono::milliseconds LOBBY_RESEND_RATE {500};

    inline constexpr float LOBBY_ROW_WIDTH = 1000.f;
    inline constexpr float LOBBY_ROW_HEIGHT = 80.f;
    inline constexpr std::size_t LOBBY_ROW_TILE_OTHER = 0;
    inline constexpr std::size_t LOBBY_ROW_TILE_ME = 1;

    inline constexpr std::size_t LOBBY_MAX_ROWS = 5;
    inline constexpr float LOBBY_ROW_GAP = 12.f;

    inline constexpr float LOBBY_TITLE_Y = 100.f;
    inline constexpr float LOBBY_TITLE_SIZE = 64.f;
    inline constexpr float LOBBY_SUBTITLE_Y = 200.f;
    inline constexpr float LOBBY_FIRST_ROW_Y = 300.f;
    inline constexpr float LOBBY_STATUS_Y = 750.f;
    inline constexpr float LOBBY_ROW_PADDING = 30.f;
    inline constexpr float LOBBY_NAME_SIZE = 44.f;
    inline constexpr float LOBBY_HOST_SIZE = 32.f;
    inline constexpr float LOBBY_INFO_SIZE = 30.f;
    inline constexpr float LOBBY_BUTTONS_Y = 800.f;

    struct LobbyItem {};

    enum class LobbyButtonId { Ready, Start, Leave };

    struct LobbyButton {
        LobbyButtonId id;
    };

    struct LobbyRow {
        std::size_t row;
    };

    enum class LobbyTextId { Subtitle, Status, RowName, RowHost, RowStatus };

    struct LobbyText {
        LobbyTextId id;
        std::size_t row = 0;
    };

    struct LobbyClient {
        bool ready = false;
        bool startRequested = false;
        bool shown = false;
        bool inGame = false;
        std::chrono::steady_clock::time_point lastInfo {};
        std::chrono::steady_clock::time_point lastStart {};
    };

    void createLobby(GameManager &gm);
    void destroyLobby(GameManager &gm);
    void returnToMenu(GameManager &gm);
    void systemLobby(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_LOBBY_HPP_ */
