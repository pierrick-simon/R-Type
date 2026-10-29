/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** DestroyProtocol
*/

#include "DestroyProtocol.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"

namespace RType {

    /**
     * The quad drawn this frame is only rendered at the next frame's
     * EndDrawing, after the destroy has unloaded its texture: the stale
     * texture id would then be drawn with another entity's texture. Making
     * it transparent hides that last quad.
     */
    template<typename T>
    static void hide(GameManager &gm, Entity e)
    {
        if (gm.hasComponent<T>(e))
            gm.getComponent<T>(e).color.a = 0;
    }

    void DestroyProtocol::manageDestroy(
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
        if (!gm.hasEntity(entity))
            return;
        hide<Sprite2D>(gm, entity);
        hide<SpriteSheet2D>(gm, entity);
        hide<Text2D>(gm, entity);
        gm.destroyEntity(entity);
    }

    void DestroyProtocol::sendDestroy(
        const std::vector<uint32_t> &ids, ServerManager &s, uint32_t id)
    {
        s.sendToClients(ids, OpcodeRType::DESTROY, serializeDestroy({id}));
    }

    std::vector<uint8_t> DestroyProtocol::serializeDestroy(
        const DestroyPayload &e)
    {
        std::vector<uint8_t> out(DestroyPayload::SIZE);

        put32(&out[0], e.id);
        return out;
    }

    std::optional<DestroyPayload> DestroyProtocol::parseDestroy(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != DestroyPayload::SIZE)
            return std::nullopt;
        return DestroyPayload {get32(payload.data())};
    }

} // namespace RType
