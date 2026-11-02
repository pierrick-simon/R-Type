/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MoveProtocol
*/

#include "MoveProtocol.hpp"
#include <cmath>
#include "Position2D.hpp"

namespace RType {

    void MoveProtocol::manageMove(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm)
            return;
        auto e = parseMove(payload);
        if (!e)
            return;
        auto it = ctx.map.find(e->id);
        if (it == ctx.map.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasEntity(it->second)) {
            ctx.map.erase(it);
            return;
        }
        if (!gm.hasComponent<Position2D>(it->second) ||
            !gm.hasComponent<Velocity2D>(it->second))
            return;
        auto &pos = gm.getComponent<Position2D>(it->second);
        auto &v = gm.getComponent<Velocity2D>(it->second);
        pos.x = static_cast<float>(e->posX);
        pos.y = static_cast<float>(e->posY);
        v.x = static_cast<float>(e->vX);
        v.y = static_cast<float>(e->vY);
    }

    void MoveProtocol::sendMove(const std::vector<uint32_t> &ids,
        ServerManager &s, uint32_t id, const Position2D &pos,
        const Velocity2D &v)
    {
        MovePayload p {id,
            static_cast<int32_t>(std::lround(pos.x)),
            static_cast<int32_t>(std::lround(pos.y)),
            static_cast<int32_t>(std::lround(v.x)),
            static_cast<int32_t>(std::lround(v.y))};
        s.sendToClients(ids, OpcodeRType::MOVE, serializeMove(p));
    }

    std::vector<uint8_t> MoveProtocol::serializeMove(const MovePayload &e)
    {
        std::vector<uint8_t> out(MovePayload::SIZE);

        put32(&out[0], e.id);
        put32(&out[4], static_cast<uint32_t>(e.posX));
        put32(&out[8], static_cast<uint32_t>(e.posY));
        put32(&out[12], static_cast<uint32_t>(e.vX));
        put32(&out[16], static_cast<uint32_t>(e.vY));
        return out;
    }

    std::optional<MovePayload> MoveProtocol::parseMove(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != MovePayload::SIZE)
            return std::nullopt;
        MovePayload e {};
        e.id = get32(payload.data());
        e.posX = static_cast<int32_t>(get32(payload.data() + 4));
        e.posY = static_cast<int32_t>(get32(payload.data() + 8));
        e.vX = static_cast<int32_t>(get32(payload.data() + 12));
        e.vY = static_cast<int32_t>(get32(payload.data() + 16));
        return e;
    }

} // namespace RType
