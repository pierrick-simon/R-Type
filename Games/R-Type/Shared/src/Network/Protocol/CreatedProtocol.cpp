/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CreatedProtocol
*/

#include "CreatedProtocol.hpp"
#include "ServerManager.hpp"
#include "UpdateClientEntity.hpp"
#include "Velocity2D.hpp"

namespace RType {
    std::optional<Entity> CreatedProtocol::getEntity(
        GameManager &gm, uint32_t id)
    {
        std::optional<Entity> entity;
        for (auto [e, u] : gm.view<UpdateClientEntity>().getVector()) {
            if (u.getId() == id) {
                entity = e;
                break;
            }
        }
        return entity;
    }

    void CreatedProtocol::manageCreated(const Header &header,
        const std::vector<uint8_t> &payload, ServerManager &server)
    {
        auto &ctx = server.getContext();
        if (!ctx.gm)
            return;
        auto e = parseCreated(payload);
        if (!e)
            return;
        auto &gm = ctx.gm->get();
        auto entity = getEntity(ctx.gm->get(), e->id);
        if (!entity)
            return;
        if (gm.hasEntity(entity.value())) {
            if (gm.hasComponent<UpdateClientEntity>(entity.value())) {
                auto &comp =
                    gm.getComponent<UpdateClientEntity>(entity.value());
                comp.initUpdateEntity(
                    gm, entity.value(), server, header.senderId);
            }
        }
    }

    void CreatedProtocol::sendCreated(ClientManager &c, uint32_t id)
    {
        c.send(OpcodeRType::CREATE, serializeCreated({id}));
    }

    std::vector<uint8_t> CreatedProtocol::serializeCreated(
        const CreatedPayload &e)
    {
        std::vector<uint8_t> out(CreatedPayload::SIZE);

        put32(&out[0], e.id);
        return out;
    }

    std::optional<CreatedPayload> CreatedProtocol::parseCreated(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != CreatedPayload::SIZE)
            return std::nullopt;
        return CreatedPayload {get32(payload.data())};
    }
} // namespace RType
