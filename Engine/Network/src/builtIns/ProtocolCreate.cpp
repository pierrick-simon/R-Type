/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolCreate
*/

#include "ProtocolCreate.hpp"

namespace RType {

    void ProtocolCreate::manageCreate(const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() < ProtocolEntity::SIZE)
            return;
        auto e = parseEntity(payload);
        if (e->type == EntityType::SPRITE)
            manageCreateSprite2D(e.value(), ctx);
        else if (e->type == EntityType::SPRITESHEET)
            manageCreateSpriteSheet(e.value(), ctx);
        else if (e->type == EntityType::SOUND)
            manageCreateSound(e.value(), ctx);
        else if (e->type == EntityType::TEXT)
            manageCreateSound(e.value(), ctx);
    }

    void ProtocolCreate::manageCreateSprite2D(ProtocolEntity &e, Context &ctx)
    {
        auto &gm = ctx.gm->get();
        auto asset = gm.getAssetsMap().find(e.id);
        
        if (asset == gm.getAssetsMap().end())
            return;
        auto s = ctx.gm->get().createEntity();
        gm.addComponent<RType::Position2D>(s, e.posX, e.posY);
        gm.addComponent<RType::Texture2D>(s, asset->second);
        ctx.map.emplace(e.id, s);
    }


std::vector<uint8_t> serializeEntity(const ProtocolEntity &e)
{
    std::vector<uint8_t> out(ProtocolEntity::SIZE);

    put32(&out[0], e.id);
    out[4] = e.type;
    put16(&out[5], e.assetId);
    put32(&out[7], e.posX);
    put32(&out[11], e.posY);
    return out;
}

std::optional<ProtocolEntity> parseEntity(const std::vector<uint8_t> &payload)
{
    ProtocolEntity e = {};

    if (payload.size() != ProtocolEntity::SIZE)
        return std::nullopt;
    e.id = get32(payload.data());
    e.type = payload[4];
    e.assetId = get16(payload.data() + 5);
    e.posX = get32(payload.data() + 7);
    e.posY = get32(payload.data() + 11);
    return e;
}


} //namespace RType
