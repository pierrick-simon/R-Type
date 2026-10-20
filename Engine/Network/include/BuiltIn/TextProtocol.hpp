/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TextProtocol
*/

#ifndef TEXTPROTOCOL_HPP_
    #define TEXTPROTOCOL_HPP_

#include <optional>
#include <string>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Payload of a TEXT packet (server -> client).
     *
     * Layout: entityId (4) | textLength (2) | text (textLength, UTF-8).
     */
    struct TextPayload {
        static constexpr std::size_t MIN_SIZE = 4 + 2;
        uint32_t id = 0;
        std::string text;
    };

    class TextProtocol {
    public:
        TextProtocol() = default;

        /** @brief Client handler: sets the Text2D string of the entity. */
        static void manageText(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helper: sendToClients a new text for entity `id`. */
        static void sendText(const std::vector<uint32_t> &ids, ServerManager &s,
            uint32_t id, const std::string &text);

        static std::vector<uint8_t> serializeText(const TextPayload &e);
        static std::optional<TextPayload> parseText(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !TEXTPROTOCOL_HPP_ */
