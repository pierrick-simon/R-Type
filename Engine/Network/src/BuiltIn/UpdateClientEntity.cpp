/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateClientEntity
*/

#include "UpdateClientEntity.hpp"
#include <unordered_set>
#include "ProtocolCreate.hpp"
#include "ProtocolDestroy.hpp"
#include "ProtocolMove.hpp"
#include "ProtocolSet.hpp"
#include "ProtocolText.hpp"
#include "SetClientEntity.hpp"

namespace RType {
    static void removeCommon(
        std::vector<uint32_t> &a, const std::vector<uint32_t> &b)
    {
        std::unordered_set<uint32_t> toRemove(b.begin(), b.end());

        a.erase(std::remove_if(a.begin(),
                    a.end(),
                    [&](uint32_t v) { return toRemove.count(v) > 0; }),
            a.end());
    }

    UpdateClientEntity::UpdateClientEntity(
        EntityType type, int asset, bool active) :
        _type(type), asset(asset), active(active)
    {
        static uint32_t id = 0;
        _id = id;
        id++;
    };

    void UpdateClientEntity::systemUpdateEntityPos(
        GameManager &gm, ServerManager &server)
    {
        for (auto [_, pos, update] :
            gm.view<Position2D, UpdateClientEntity>().getVector()) {
            if (!pos.getUpdate() || !update.active)
                continue;
            pos.setUpdate(false);
            ProtocolMove::sendMove(update._links, server, update.getId(), pos);
        }
    }

    void UpdateClientEntity::systemCreateClientEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [entity, update] :
            gm.view<UpdateClientEntity>().getVector()) {
            ProtocolEntity pe = {};
            if (update.active = true)
                continue;
            checkConnectedClient(server, update);
            try {
                auto tmp = gm.getComponent<Position2D>(entity);
                pe.posX = uint32_t(tmp.x());
                pe.posY = uint32_t(tmp.y());
            } catch (NoSuchEntity &e) {
            }
            pe.assetId = update.asset;
            pe.id = update.getId();
            pe.type = update._type;
            removeCommon(update.create, update._links);
            ProtocolCreate::sendCreate(update.create, server, pe);
            setCreateEntity(gm, entity, server, update);
            update._links.insert(update._links.end(),
                update.create.begin(),
                update.create.end());
            update.create.clear();
        }
    }

    void UpdateClientEntity::setCreateEntity(GameManager &gm, Entity entity,
        ServerManager &server, UpdateClientEntity &update)
    {
        SetClientEntity::systemSetAllEntity<Sprite2D, SpriteSheet2D, Text2D>(
            gm, entity, server, update);
        try {
            auto text = gm.getComponent<Text2D>(entity);
            ProtocolText::sendText(
                update.create, server, update._id, text.str());
        } catch (NoSuchEntity &e) {
        }
    }

    void UpdateClientEntity::systemUpdateTextEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [_, text, update] :
            gm.view<Text2D, UpdateClientEntity>().getVector()) {
            if (!update.active || text.getUpdateText())
                continue;
            ProtocolText::sendText(
                update._links, server, update._id, text.str());
            text.setUpdateText(false);
        }
    }

    void UpdateClientEntity::removeLink(uint32_t id)
    {
        for (auto iter = _links.begin(); iter != _links.end(); iter++) {
            if (*iter != id)
                continue;
            _links.erase(iter);
            return;
        }
    }

    void UpdateClientEntity::checkConnectedClient(
        ServerManager &server, UpdateClientEntity &update)
    {
        std::unordered_set<uint32_t> lookup(
            update._links.begin(), update._links.end());

        for (const auto &[key, _] : server.getClients()) {
            if (lookup.find(key) == lookup.end())
                update.create.push_back(key);
        }
    }

    void UpdateClientEntity::destroyUpdateEntity(
        GameManager &gm, Entity e, ServerManager &server)
    {
        try {
            auto &update = gm.getComponent<UpdateClientEntity>(e);
            ProtocolDestroy::sendDestroy(
                update.getLinks(), server, update.getId());
            gm.destroyEntity(e);
        } catch (NoSuchEntity &e) {
        }
    }
}; // namespace RType
