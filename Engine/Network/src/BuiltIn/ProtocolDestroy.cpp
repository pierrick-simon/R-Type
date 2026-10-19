/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolDestroy
*/

#include "ProtocolDestroy.hpp"

namespace RType {

    void ProtocolDestroy::manageDestroy(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm)
            return;
        auto e = parseDestroy(payload);
        if (!e)
            return;
        auto it = ctx.map.find(e->id);
        if (it == ctx.map.end())
            return;
        auto &gm = ctx.gm->get();
        const Entity entity = it->second;

        ctx.map.erase(it);
        if (gm.hasEntity(entity))
            gm.destroyEntity(entity);
    }

    void ProtocolDestroy::sendDestroy(ServerManager &s, uint32_t id)
    {
        s.broadcast(Opcode::DESTROY, serializeDestroy({id}));
    }

    std::vector<uint8_t> ProtocolDestroy::serializeDestroy(
        const DestroyPayload &e)
    {
        std::vector<uint8_t> out(DestroyPayload::SIZE);

        put32(&out[0], e.id);
        return out;
    }

    std::optional<DestroyPayload> ProtocolDestroy::parseDestroy(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != DestroyPayload::SIZE)
            return std::nullopt;
        return DestroyPayload {get32(payload.data())};
    }

} // namespace RType
