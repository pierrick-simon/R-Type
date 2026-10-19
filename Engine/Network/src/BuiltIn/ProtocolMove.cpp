/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolMove
*/

#include "ProtocolMove.hpp"
#include <cmath>
#include "Position2D.hpp"

namespace RType {

    void ProtocolMove::manageMove(
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
        if (!gm.hasComponent<Position2D>(it->second))
            return;
        auto &pos = gm.getComponent<Position2D>(it->second);
        pos.x = static_cast<float>(e->posX);
        pos.y = static_cast<float>(e->posY);
    }

    void ProtocolMove::sendMove(ServerManager &s, uint32_t id, float x, float y)
    {
        MovePayload p {id,
            static_cast<int32_t>(std::lround(x)),
            static_cast<int32_t>(std::lround(y))};
        s.broadcast(Opcode::MOVE, serializeMove(p));
    }

    std::vector<uint8_t> ProtocolMove::serializeMove(const MovePayload &e)
    {
        std::vector<uint8_t> out(MovePayload::SIZE);

        put32(&out[0], e.id);
        put32(&out[4], static_cast<uint32_t>(e.posX));
        put32(&out[8], static_cast<uint32_t>(e.posY));
        return out;
    }

    std::optional<MovePayload> ProtocolMove::parseMove(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != MovePayload::SIZE)
            return std::nullopt;
        MovePayload e {};
        e.id = get32(payload.data());
        e.posX = static_cast<int32_t>(get32(payload.data() + 4));
        e.posY = static_cast<int32_t>(get32(payload.data() + 8));
        return e;
    }

} // namespace RType
