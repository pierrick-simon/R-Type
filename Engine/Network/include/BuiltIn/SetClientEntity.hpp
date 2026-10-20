/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SetClientEntity
*/

#ifndef SETCLIENTENTITY_HPP_
#define SETCLIENTENTITY_HPP_

#include <concepts>
#include <optional>
#include <vector>
#include "GameManager.hpp"
#include "Property.hpp"
#include "SetProtocol.hpp"
#include "ServerManager.hpp"
#include "UpdateClientEntity.hpp"

namespace RType {

    /** @brief A component whose modified properties can be sent as SET packets.
     */
    template<typename C>
    concept NetworkSynced = requires(C &c, const C &cc, Property p) {
        {
            cc.getUpdate()
        } -> std::convertible_to<const std::vector<Property> &>;
        {
            cc.getDefaultUpdate()
        } -> std::convertible_to<const std::vector<Property> &>;
        { cc.getProperty(p) } -> std::same_as<std::optional<SetValue>>;
        c.clearUpdate();
    };

    struct SetClientEntity {
        /**
         * @brief Server system: sends one SET packet per default property of
         * one entity owning a component @p C and an UpdateClientEntity.
         *
         * @tparam C      Component type (Text2D, Sprite2D, SpriteSheet2D...).
         * @param gm      GameMaster
         * @param entity  Entity
         * @param server  The server used to broadcast the packets.
         * @param update  Component UpdateClientEntity
         */
        template<NetworkSynced C>
        static void systemSetEntity(GameManager &gm, Entity entity,
            ServerManager &server, UpdateClientEntity &update)
        {
            try {
                auto &comp = gm.getComponent<C>(entity);
                for (Property prop : comp.getDefaultUpdate()) {
                    auto value = comp.getProperty(prop);
                    if (!value)
                        return;
                    SetPayload p {update.getId(), prop, *value};
                    SetProtocol::sendSet(update.create, server, p);
                }
            } catch (NoSuchEntity &) {
            }
        }

        /** @brief Sends the default properties of every listed component type.
         */
        template<NetworkSynced... Cs>
        static void systemSetAllEntity(GameManager &gm, Entity entity,
            ServerManager &server, UpdateClientEntity &update)
        {
            (systemSetEntity<Cs>(gm, entity, server, update), ...);
        }

        /**
         * @brief Server system: sends one SET packet per modified property of
         * one entity owning a component @p C and an UpdateClientEntity.
         *
         * @tparam C      Component type (Text2D, Sprite2D, SpriteSheet2D...).
         * @param server  The server used to broadcast the packets.
         * @param comp    Component C
         * @param update  Component UpdateClientEntity
         */
        template<NetworkSynced C>
        static void systemUpdateEntity(
            ServerManager &server, C &comp, UpdateClientEntity &update)
        {
            if (!update.active)
                return;
            for (Property prop : comp.getUpdate()) {
                auto value = comp.getProperty(prop);
                if (!value)
                    return;
                SetPayload p {update.getId(), prop, *value};
                SetProtocol::sendSet(update.getLinks(), server, p);
            }
            comp.clearUpdate();
        }

        /**
         * @brief Server system: sends one SET packet per modified property of
         * every entity owning a component @p C and an UpdateClientEntity.
         *
         * @tparam C      Component type (Text2D, Sprite2D, SpriteSheet2D...).
         * @param gm      The game manager holding the entities.
         * @param server  The server used to broadcast the packets.
         */
        template<NetworkSynced C>
        static void systemUpdateEntitys(GameManager &gm, ServerManager &server)
        {
            for (auto &&[_, comp, update] :
                gm.view<C, UpdateClientEntity>().getVector()) {
                systemUpdateEntity(server, comp, update);
            }
        }

        /** @brief Same, for several component types at once. */
        template<NetworkSynced... Cs>
        static void systemUpdatesAllEntity(
            GameManager &gm, ServerManager &server)
        {
            (systemUpdateEntitys<Cs>(gm, server), ...);
        }
    };

} // namespace RType
#endif /* !SETCLIENTENTITY_HPP_ */
