/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyListPage
*/

#ifndef LOBBYLISTPAGE_HPP_
#define LOBBYLISTPAGE_HPP_

#include <chrono>
#include <string>

#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr std::chrono::milliseconds LOBBY_LIST_RESEND_RATE {500};

    inline constexpr float LOBBY_LIST_TITLE_Y = 100.f;
    inline constexpr float LOBBY_LIST_TITLE_SIZE = 64.f;
    inline constexpr float LOBBY_LIST_FIRST_ROW_Y = 190.f;
    inline constexpr float LOBBY_LIST_ROW_WIDTH = 1300.f;
    inline constexpr float LOBBY_LIST_ROW_HEIGHT = 70.f;
    inline constexpr float LOBBY_LIST_ROW_GAP = 12.f;
    inline constexpr float LOBBY_LIST_TEXT_SIZE = 34.f;
    inline constexpr float LOBBY_LIST_PLAYERS_X = 360.f;
    inline constexpr float LOBBY_LIST_NAME_MAX_WIDTH = 290.f;
    inline constexpr std::size_t LOBBY_LIST_MAX_ROWS = 8;

    inline constexpr float LOBBY_LIST_VISIBLE_TOP = 180.f;
    inline constexpr float LOBBY_LIST_VISIBLE_BOTTOM = 830.f;
    inline constexpr int LOBBY_LIST_SCROLL_STEP = 5;
    inline constexpr float CREATE_POPUP_WIDTH = 900.f;
    inline constexpr float CREATE_POPUP_HEIGHT = 440.f;
    inline constexpr float CREATE_POPUP_PADDING = 40.f;

    struct LobbyListItem {};

    struct LobbyListFrame {};

    struct CreateLobbyButton {};

    struct JoinLobbyButton {};

    struct CreatePopupItem {};

    struct CreatePopupConfirm {};

    struct LobbyListClient {
        bool shown = false;
        bool wanted = false;
        bool createRequested = false;
        std::string createName;
        bool popupOpen = false;
        uint16_t selectedPort = 0;
        uint16_t joinPort = 0;
        std::chrono::steady_clock::time_point lastRequest {};
    };

    inline int scroll = 0;

    void connectToMaster(GameManager &gm, const std::string &host, int port);

    void openLobbyList(GameManager &gm);
    void createLobbyListPage(GameManager &gm);
    void destroyLobbyListPage(GameManager &gm);
    void systemLobbyListPage(GameManager &gm);
    void systemDrawLobbyList(GameManager &gm);

} // namespace RType::Client

#endif /* !LOBBYLISTPAGE_HPP_ */
