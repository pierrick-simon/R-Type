/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolSet
*/

#ifndef PROTOCOL_SET_HPP_
    #define PROTOCOL_SET_HPP_

#include <cstdint>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "Property.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Payload of a SET packet (server -> client).
     *
     * Layout: entityId (4) | prop (1) | value (valueSize(prop)).
     */
    struct SetPayload {
        static constexpr std::size_t HEADER_SIZE = 4 + 1;
        uint32_t id;
        Property prop;
        SetValue value;
    };

    class ProtocolSet {
    public:
        ProtocolSet() = default;

        /** @brief Client handler: applies the property to the entity. */
        static void manageSet(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helpers: broadcast one property change. */
        static void sendSet(const std::vector<uint32_t> &ids, ServerManager &s,
            const SetPayload &p);

        /** @return Size in bytes of the value of `prop`. */
        static std::size_t valueSize(Property prop);

        static std::vector<uint8_t> serializeSet(const SetPayload &e);
        static std::optional<SetPayload> parseSet(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !PROTOCOL_SET_HPP_ */
