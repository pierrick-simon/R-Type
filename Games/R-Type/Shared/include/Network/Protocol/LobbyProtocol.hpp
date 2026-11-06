/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyProtocol
*/

#ifndef LOBBYPROTOCOL_HPP_
    #define LOBBYPROTOCOL_HPP_

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "ClientManager.hpp"
#include "Context.hpp"
#include "Network.hpp"

namespace RType {

    inline constexpr std::size_t LOBBY_NAME_MAX = 16;

    struct LobbyMember {
        std::string name;
        bool ready = false;
        bool startRequested = false;
        bool restartRequested = false;
        bool returnRequested = false;
    };

    struct LobbyPlayerInfo {
        uint16_t id = 0;
        bool ready = false;
        std::string name;
    };

    struct LobbyState {
        bool received = false;
        bool started = false;
        bool gameOver = false;
        uint16_t hostId = 0;
        std::vector<LobbyPlayerInfo> players;
        std::string lobbyName;

        [[nodiscard]] bool allReady() const
        {
            if (players.empty() ||
                std::ranges::all_of(players.begin(),
                    players.end(),
                    [](const LobbyPlayerInfo &p) { return !p.ready; }))
                return false;
            return true;
        }
    };

    class LobbyProtocol {
    public:
        static void manageInfo(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageStart(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageState(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageRestart(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageReturn(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        static void sendInfo(
            ClientManager &c, const std::string &name, bool ready);
        static void sendStart(ClientManager &c);
        static void sendRestart(ClientManager &c);
        static void sendReturn(ClientManager &c);

        static std::vector<uint8_t> serializeState(const LobbyState &s);
        static std::optional<LobbyState> parseState(
            const std::vector<uint8_t> &payload);

        static std::string sanitizeName(const std::string &name);
    };

} // namespace RType

#endif /* !LOBBYPROTOCOL_HPP_ */
