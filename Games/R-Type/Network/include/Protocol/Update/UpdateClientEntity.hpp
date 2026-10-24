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
#include "Clock.hpp"
#include "CreateProtocol.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    using UpdateClass = std::variant<Sprite2D, SpriteSheet2D, Text2D>;

    class UpdateClientEntity {
    public:
        UpdateClientEntity(EntityType type, int asset, bool active = true,
            std::size_t rate = 30);

        std::vector<uint32_t> create;

        int asset;
        bool active;
        std::size_t rate;

        static void systemUpdateEntityPos(
            GameManager &gm, ServerManager &server);
        static void systemCreateClientEntity(
            GameManager &gm, ServerManager &server);
        static void systemUpdateTextEntity(
            GameManager &gm, ServerManager &server);

        void initUpdateEntity(
            GameManager &gm, Entity entity, ServerManager &server, uint32_t id);

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

        [[nodiscard]] std::vector<uint32_t> &getInits()
        {
            return _inits;
        }

        [[nodiscard]] bool getUpdate() const
        {
            return _update;
        }

        void removeLink(uint32_t id);
        void removeInit(uint32_t id);

        static void destroyUpdateEntity(
            GameManager &gm, Entity e, ServerManager &server);

        static void systemMasterUpdateClientEntitySleep(
            GameManager &gm, MasterClock &clock);

    private:
        uint32_t _id;
        std::vector<uint32_t> _links;
        std::vector<uint32_t> _inits;
        EntityType _type;
        std::chrono::milliseconds _sleep = std::chrono::milliseconds(0);
        bool _update = true;

        static void setCreateEntity(GameManager &gm, Entity entity,
            ServerManager &server, UpdateClientEntity &update);
        static void checkConnectedClient(
            ServerManager &server, UpdateClientEntity &update);
    };
}; // namespace RType

#endif /* !UPDATECLIENTENTITY_HPP_ */
