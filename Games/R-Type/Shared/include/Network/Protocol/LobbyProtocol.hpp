/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyProtocol
*/

#ifndef LOBBYPROTOCOL_HPP_
    #define LOBBYPROTOCOL_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Context.hpp"
#include "Network.hpp"
#include "ClientManager.hpp"

namespace RType {

    inline constexpr std::size_t LOBBY_NAME_MAX = 16;

    struct LobbyMember {
        std::string name;
        bool ready = false;
        bool startRequested = false;
    };

    struct LobbyPlayerInfo {
        uint16_t id = 0;
        bool ready = false;
        std::string name;
    };

    struct LobbyState {
        bool received = false;
        bool started = false;
        uint16_t hostId = 0;
        std::vector<LobbyPlayerInfo> players;
        [[nodiscard]] bool allReady() const
        {
            if (players.empty())
                return false;
            for (const auto &p : players)
                if (!p.ready)
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

        static void sendInfo(ClientManager &c, const std::string &name, bool ready);
        static void sendStart(ClientManager &c);

        static std::vector<uint8_t> serializeState(const LobbyState &s);
        static std::optional<LobbyState> parseState(const std::vector<uint8_t> &payload);

        static std::string sanitizeName(const std::string &name);
    };

} // namespace RType

#endif /* !LOBBYPROTOCOL_HPP_ */
