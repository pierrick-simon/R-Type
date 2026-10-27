/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Destroy
*/

#ifndef DESTROY_HPP_
#define DESTROY_HPP_

#include "GameManager.hpp"
#include "ServerManager.hpp"
#include "UpdateClientEntity.hpp"

namespace RType {

    inline void destroyNetworked(GameManager &gm, Entity e)
    {
        if (gm.hasComponent<UpdateClientEntity>(e)) {
            for (auto [_, server] : gm.view<ServerManager>().getVector()) {
                UpdateClientEntity::destroyUpdateEntity(gm, e, server);
                return;
            }
        }
        gm.destroyEntity(e);
    }

} // namespace RType

#endif /* !DESTROY_HPP_ */
