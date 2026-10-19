/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolDestroy
*/

#ifndef PROTOCOL_DESTROY_HPP_
    #define PROTOCOL_DESTROY_HPP_

#include <optional>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Payload of a DESTROY packet (server -> client).
     *
     * Layout: entityId (4).
     */
    struct DestroyPayload {
        static constexpr std::size_t SIZE = 4;
        uint32_t id;
    };

    class ProtocolDestroy {
    public:
        ProtocolDestroy() = default;

        /** @brief Client handler: destroys the entity bound to the network id. */
        void manageDestroy(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helper: broadcasts the destruction of entity `id`. */
        static void sendDestroy(ServerManager &s, uint32_t id);

        static std::vector<uint8_t> serializeDestroy(const DestroyPayload &e);
        static std::optional<DestroyPayload> parseDestroy(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !PROTOCOL_DESTROY_HPP_ */
