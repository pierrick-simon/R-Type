/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateClientEntity
*/

#ifndef UPDATECLIENTENTITY_HPP_
#define UPDATECLIENTENTITY_HPP_

#include <concepts>
#include <optional>
#include <variant>
#include "CreateProtocol.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    using UpdateClass = std::variant<Sprite2D, SpriteSheet2D, Text2D>;

    class UpdateClientEntity {
    public:
        UpdateClientEntity(EntityType type, int asset, bool active = true);

        std::vector<uint32_t> create;

        int asset;
        bool active;

        static void systemUpdateEntityPos(
            GameManager &gm, ServerManager &server);
        static void systemCreateClientEntity(
            GameManager &gm, ServerManager &server);
        static void systemUpdateTextEntity(
            GameManager &gm, ServerManager &server);

        [[nodiscard]] uint32_t getId() const
        {
            return _id;
        }

        [[nodiscard]] const std::vector<uint32_t> &getLinks() const
        {
            return _links;
        }

        [[nodiscard]] std::vector<uint32_t> &getLinks()
        {
            return _links;
        }

        void removeLink(uint32_t id);

        static void destroyUpdateEntity(
            GameManager &gm, Entity e, ServerManager &server);

    private:
        uint32_t _id;
        std::vector<uint32_t> _links;
        EntityType _type;

        static void setCreateEntity(GameManager &gm, Entity entity,
            ServerManager &server, UpdateClientEntity &update);
        static void checkConnectedClient(
            ServerManager &server, UpdateClientEntity &update);
    };
}; // namespace RType

#endif /* !UPDATECLIENTENTITY_HPP_ */
