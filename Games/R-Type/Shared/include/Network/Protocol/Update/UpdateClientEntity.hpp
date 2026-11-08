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
#include <vector>
#include "Clock.hpp"
#include "CreateProtocol.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {
    class UpdateClientEntity {
    public:
        struct Sound {
            Sound(int asset) :
                asset(asset) {};
            int asset;
            std::vector<uint32_t> sent;
        };

        UpdateClientEntity(EntityType type, int asset, GameManager &gm,
            Entity entity, const std::optional<Sound> &sound = std::nullopt);

        std::vector<uint32_t> create;

        int asset;
        std::size_t rate = 30;

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
        std::optional<Sound> _sound;

        static void checkConnectedClient(
            ServerManager &server, UpdateClientEntity &update);
    };
}; // namespace RType

#endif /* !UPDATECLIENTENTITY_HPP_ */
