/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterProtocol
*/

#ifndef MASTERPROTOCOL_HPP_
#define MASTERPROTOCOL_HPP_

#include <cstdint>
#include <vector>
#include "ClientManager.hpp"
#include "Context.hpp"
#include "Network.hpp"

namespace RType {

    class ServerManager;

    struct LobbyRequest {
        bool createRequested = false;
    };

    struct LobbyPortInfo {
        uint16_t port = 0;
    };

    class MasterProtocol {
    public:
        static void manageCreateLobby(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageLobbyPort(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        static void sendCreateLobby(ClientManager &c);
        static void sendLobbyPort(
            ServerManager &s, uint32_t clientId, uint16_t port);
    };

} // namespace RType

#endif /* !MASTERPROTOCOL_HPP_ */
