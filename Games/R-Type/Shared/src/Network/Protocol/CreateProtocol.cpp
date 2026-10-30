/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CreateProtocol
*/

#include "CreateProtocol.hpp"
#include "Assets.hpp"
#include "Clock.hpp"
#include "Friction2D.hpp"
#include "Listener.hpp"
#include "PlayerPhysics.hpp"
#include "ServerManager.hpp"
#include "Velocity2D.hpp"

namespace RType {

    void CreateProtocol::manageCreate(const Header &header,
        const std::vector<uint8_t> &payload, ClientManager &client)
    {
        auto &ctx = client.getContext();
        if (!ctx.gm || payload.size() < ProtocolEntity::SIZE)
            return;
        auto e = parseEntity(payload);
        if (!e || ctx.map.contains(e->id))
            return;
        switch (e->type) {
            case EntityType::SPRITE:
                manageCreateSprite2D(e.value(), ctx);
                break;
            case EntityType::SPRITESHEET:
                manageCreateSpriteSheet(e.value(), ctx);
                break;
            case EntityType::SOUND:
                manageCreateSound(e.value(), ctx);
                break;
            case EntityType::TEXT_ENTITY:
                manageCreateText2D(e.value(), ctx);
                break;
            default:
                return;
        }
        client.addCreatedEntity(e.value().id);
    }

    void CreateProtocol::manageCreateSprite2D(ProtocolEntity &e, Context &ctx)
    {
        auto &gm = ctx.gm->get();
        auto asset = gm.getAssetsMap().find(e.assetId);

        if (asset == gm.getAssetsMap().end())
            return;
        auto s = ctx.gm->get().createEntity();
        gm.addComponent<RType::Sprite2D>(s);
        gm.addComponent<RType::Position2D>(s, e.posX, e.posY);
        gm.addComponent<RType::Velocity2D>(s);
        gm.addComponent<RType::Texture2D>(s, asset->second);
        ctx.map.emplace(e.id, s);
    }

    void CreateProtocol::manageCreateSpriteSheet(
        ProtocolEntity &e, Context &ctx)
    {
        auto &gm = ctx.gm->get();
        auto asset = gm.getAssetsMap().find(e.assetId);

        if (asset == gm.getAssetsMap().end())
            return;
        auto s = ctx.gm->get().createEntity();
        RType::SpriteSheet2D sheet;
        sheet.color.a = 0;
        gm.addComponent<RType::SpriteSheet2D>(s, sheet);
        gm.addComponent<RType::Position2D>(s, e.posX, e.posY);
        gm.addComponent<RType::Velocity2D>(s);
        gm.addComponent<RType::Texture2D>(s, asset->second);
        if (e.assetId >= Asset::PLAYER_SHEET_1 &&
            e.assetId <= Asset::PLAYER_SHEET_5) {
            gm.addComponent<RType::Friction2D>(s, PLAYER_FRICTION);
            gm.addComponent<RType::Clock>(s);
        }
        ctx.map.emplace(e.id, s);
    }

    void CreateProtocol::manageCreateSound(ProtocolEntity &e, Context &ctx)
    {
        auto &gm = ctx.gm->get();
        auto asset = gm.getAssetsMap().find(e.assetId);

        if (asset == gm.getAssetsMap().end())
            return;
        for (auto [_, l] : gm.view<Listener>().getVector()) {
            auto s = ctx.gm->get().createEntity();
            gm.addComponent<RType::Sound>(s, l, asset->second);
            gm.addComponent<RType::Position2D>(s, e.posX, e.posY);
            gm.addComponent<RType::Velocity2D>(s);
            ctx.map.emplace(e.id, s);
        }
    }

    void CreateProtocol::manageCreateText2D(ProtocolEntity &e, Context &ctx)
    {
        auto &gm = ctx.gm->get();
        auto asset = gm.getAssetsMap().find(e.assetId);

        if (asset == gm.getAssetsMap().end())
            return;
        auto s = ctx.gm->get().createEntity();
        gm.addComponent<RType::Text2D>(s);
        gm.addComponent<RType::Font>(s, asset->second);
        gm.addComponent<RType::Position2D>(s, e.posX, e.posY);
        gm.addComponent<RType::Velocity2D>(s);
        ctx.map.emplace(e.id, s);
    }

    std::vector<uint8_t> CreateProtocol::serializeEntity(
        const ProtocolEntity &e)
    {
        std::vector<uint8_t> out(ProtocolEntity::SIZE);

        put32(&out[0], e.id);
        out[4] = e.type;
        put16(&out[5], e.assetId);
        put32(&out[7], e.posX);
        put32(&out[11], e.posY);
        return out;
    }

    std::optional<ProtocolEntity> CreateProtocol::parseEntity(
        const std::vector<uint8_t> &payload)
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

    void CreateProtocol::sendCreate(const std::vector<uint32_t> &ids,
        ServerManager &server, ProtocolEntity &pe)
    {
        server.sendToClients(ids, OpcodeRType::CREATE, serializeEntity(pe));
    }

} // namespace RType
