/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterProtocol
*/

#ifndef MASTERPROTOCOL_HPP_
#define MASTERPROTOCOL_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include "ClientManager.hpp"
#include "Context.hpp"
#include "Network.hpp"

namespace RType {

    class ServerManager;

    struct LobbyRequest {
        bool createRequested = false;
        std::string name;
        uint16_t joinPort = 0;
    };

    struct LobbyPortInfo {
        uint16_t port = 0;
        bool refused = false;
    };

    inline constexpr uint8_t MAX_LOBBY_PLAYERS = 5;

    inline constexpr std::size_t MAX_LISTED_LOBBIES = 40;

    struct LobbyEntry {
        uint16_t port = 0;
        uint8_t players = 0;
        bool started = false;
        std::string name;

        bool operator==(const LobbyEntry &other) const = default;
    };

    struct LobbyListState {
        bool received = false;
        std::vector<LobbyEntry> lobbies;
    };

    class MasterProtocol {
    public:
        static void manageCreateLobby(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageLobbyPort(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageJoinLobby(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageLobbyList(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        static void sendCreateLobby(ClientManager &c, const std::string &name);
        static void sendJoinLobby(ClientManager &c, uint16_t port);
        static void sendLobbyPort(
            ServerManager &s, uint32_t clientId, uint16_t port);
        static std::vector<uint8_t> serializeLobbyList(
            const std::vector<LobbyEntry> &lobbies);
    };

} // namespace RType

#endif /* !MASTERPROTOCOL_HPP_ */
