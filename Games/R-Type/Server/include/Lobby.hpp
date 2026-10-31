/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lobby
*/

#ifndef SERVER_LOBBY_HPP_
#define SERVER_LOBBY_HPP_

#include <chrono>

#include "GameManager.hpp"

namespace RType::Server {

    inline constexpr std::chrono::milliseconds LOBBY_BROADCAST_RATE {250};

    [[nodiscard]] bool isGameStarted();

    void systemLobby(GameManager &gm);

} // namespace RType::Server

#endif /* !SERVER_LOBBY_HPP_ */
