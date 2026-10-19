/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolHelper
*/

#ifndef PROTOCOL_HELPER_HPP_
    #define PROTOCOL_HELPER_HPP_

#include <optional>
#include "GameManager.hpp"
#include "Network.hpp"

namespace RType {

    /**
     * @brief Finds the local entity bound to a network id.
     *
     * Removes the id from the map if the entity no longer exists.
     * @return The entity, or std::nullopt if unknown or destroyed.
     */
    inline std::optional<Entity> findNetEntity(Context &ctx, uint32_t id)
    {
        if (!ctx.gm)
            return std::nullopt;
        auto it = ctx.map.find(id);
        if (it == ctx.map.end())
            return std::nullopt;
        if (!ctx.gm->get().hasEntity(it->second)) {
            ctx.map.erase(it);
            return std::nullopt;
        }
        return it->second;
    }

} // namespace RType

#endif /* !PROTOCOL_HELPER_HPP_ */
