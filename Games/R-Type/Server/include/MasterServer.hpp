/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterServer
*/

#ifndef MASTERSERVER_HPP_
#define MASTERSERVER_HPP_

#include <chrono>
#include <cstdint>
#include <vector>

#include "GameManager.hpp"

namespace RType::Server {

    Entity createMasterServer(GameManager &gm);

    void systemMasterRequests(GameManager &gm);

    inline constexpr std::chrono::milliseconds LOBBY_LIST_BROADCAST_RATE {500};

    struct LobbyListBroadcast {
        std::vector<uint8_t> lastPayload;
        std::chrono::steady_clock::time_point lastSent {};
    };

    void systemMasterLobbyList(GameManager &gm);

} // namespace RType::Server

#endif /* !MASTERSERVER_HPP_ */
