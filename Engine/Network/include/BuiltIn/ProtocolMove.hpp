/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolMove
*/

#ifndef PROTOCOL_MOVE_HPP_
    #define PROTOCOL_MOVE_HPP_

#include <optional>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "Position2D.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Payload of a MOVE packet (server -> client): the new position
     * of a networked entity. Positions are signed (int32, logical pixels).
     */
    struct MovePayload {
        static constexpr std::size_t SIZE = 4 + 4 + 4;
        uint32_t id;
        int32_t posX;
        int32_t posY;
    };

    class ProtocolMove {
    public:
        ProtocolMove() = default;

        /** @brief Client handler: moves the entity bound to the network id. */
        static void manageMove(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helper: sendToClients the position of entity `id`. */
        static void sendMove(const std::vector<uint32_t> &ids, ServerManager &s,
            uint32_t id, Position2D &pos);

        static std::vector<uint8_t> serializeMove(const MovePayload &e);
        static std::optional<MovePayload> parseMove(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif
