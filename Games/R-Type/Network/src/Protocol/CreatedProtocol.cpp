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
    void CreatedProtocol::manageCreated(const Header &header,
        const std::vector<uint8_t> &payload, ServerManager &server)
    {
        auto &ctx = server.getContext();
        if (!ctx.gm)
            return;
        auto e = parseCreated(payload);
        if (!e)
            return;
        auto it = ctx.map.find(e->id);
        if (it == ctx.map.end())
            return;
        auto &gm = ctx.gm->get();
        const Entity entity = it->second;

        if (gm.hasEntity(entity)) {
            if (gm.hasComponent<UpdateClientEntity>(entity)) {
                auto &comp = gm.getComponent<UpdateClientEntity>(entity);
                comp.initUpdateEntity(gm, entity, server, header.senderId);
            }
        }
        gm.destroyEntity(entity);
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
