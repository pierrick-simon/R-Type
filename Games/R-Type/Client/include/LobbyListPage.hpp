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

    /** @brief Rate at which CREATE_LOBBY is re-sent until LOBBY_PORT arrives.
     */
    inline constexpr std::chrono::milliseconds LOBBY_LIST_RESEND_RATE {500};

    /** @brief Tag of the lobby list page entities. */
    struct LobbyListItem {};

    /**
     * @brief Client-only state of the connection to the master server,
     * stored on its ClientManager entity (next to LobbyPortInfo).
     */
    struct LobbyListClient {
        bool shown = false;
        bool createRequested = false;
        std::chrono::steady_clock::time_point lastRequest {};
    };

    /**
     * @brief Connects to the master server (port 4242): once connected, the
     * lobby list page is shown.
     */
    void connectToMaster(GameManager &gm, const std::string &host, int port);

    void createLobbyListPage(GameManager &gm);
    void destroyLobbyListPage(GameManager &gm);

    /**
     * @brief Shows the page once connected to the master, sends CREATE_LOBBY
     * after a click on "Create Lobby", then switches the connection to the
     * lobby instance port received in LOBBY_PORT.
     */
    void systemLobbyListPage(GameManager &gm);

} // namespace RType::Client

#endif /* !LOBBYLISTPAGE_HPP_ */
