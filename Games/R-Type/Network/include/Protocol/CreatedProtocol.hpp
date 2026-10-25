/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#ifndef PROTOCOL_CREATED_HPP_
    #define PROTOCOL_CREATED_HPP_

#include "ClientManager.hpp"
#include "GameManager.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"

namespace RType {

    struct CreatedPayload {
        static constexpr std::size_t SIZE = 4;
        uint32_t id;
    };

    class CreatedProtocol {
    public:
        static void manageCreated(const Header &header,
            const std::vector<uint8_t> &payload, ServerManager &server);

        static void sendCreated(ClientManager &client, uint32_t id);

    private:
        static std::vector<uint8_t> serializeCreated(const CreatedPayload &e);
        static std::optional<CreatedPayload> parseCreated(
            const std::vector<uint8_t> &payload);
        static std::optional<Entity> getEntity(GameManager &gm, uint32_t id);
    };

} // namespace RType

#endif
