/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DestroyProtocol
*/

#ifndef DESTROYPROTOCOL_HPP_
    #define DESTROYPROTOCOL_HPP_

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

    class DestroyProtocol {
    public:
        DestroyProtocol() = default;

        /** @brief Client handler: destroys the entity bound to the network id.
         */
        static void manageDestroy(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helper: sendToClients the destruction of entity `id`.
         */
        static void sendDestroy(
            const std::vector<uint32_t> &ids, ServerManager &s, uint32_t id);

        static std::vector<uint8_t> serializeDestroy(const DestroyPayload &e);
        static std::optional<DestroyPayload> parseDestroy(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !DESTROYPROTOCOL_HPP_ */
