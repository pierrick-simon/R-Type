/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateClientEntity
*/

#include "UpdateClientEntity.hpp"
#include <algorithm>
#include <unordered_set>
#include "CreateProtocol.hpp"
#include "DestroyProtocol.hpp"
#include "MoveProtocol.hpp"
#include "SetClientEntity.hpp"
#include "SetProtocol.hpp"
#include "TextProtocol.hpp"
#include "UpdatePosition2D.hpp"
#include "UpdateSprite2D.hpp"
#include "UpdateSpriteSheet2D.hpp"
#include "UpdateText2D.hpp"

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
        EntityType type, int asset, bool active, std::size_t rate) :
        _type(type), asset(asset), active(active), rate(rate)
    {
        static uint32_t id = 0;
        _id = id;
        id++;
    };

    void UpdateClientEntity::systemUpdateEntityPos(
        GameManager &gm, ServerManager &server)
    {
        for (auto [entity, pos, update] :
            gm.view<UpdatePosition2D, UpdateClientEntity>().getVector()) {
            if (!pos.getUpdate() || !update.active || !update._update)
                continue;
            if (!gm.hasComponent<Velocity2D>(entity))
                MoveProtocol::sendMove(
                    update._links, server, update.getId(), pos);
            else {
                auto &v = gm.getComponent<Velocity2D>(entity);
                MoveProtocol::sendMove(
                    update._links, server, update.getId(), pos, v);
            }
            pos.setUpdate(false);
        }
    }

    void UpdateClientEntity::systemCreateClientEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [entity, update] :
            gm.view<UpdateClientEntity>().getVector()) {
            ProtocolEntity pe = {};
            removeCommon(update.create, update._links);
            removeCommon(update.create, update._inits);
            if (!update.active || !update._update || update.create.empty())
                continue;
            checkConnectedClient(server, update);
            if (gm.hasComponent<Position2D>(entity)) {
                auto tmp = gm.getComponent<Position2D>(entity);
                pe.posX = uint32_t(tmp.x);
                pe.posY = uint32_t(tmp.y);
            }
            pe.assetId = update.asset;
            pe.id = update.getId();
            pe.type = update._type;
            CreateProtocol::sendCreate(update.create, server, pe);
            update._inits.insert(update._inits.end(),
                update.create.begin(),
                update.create.end());
            update.create.clear();
        }
    }

    void UpdateClientEntity::initUpdateEntity(
        GameManager &gm, Entity entity, ServerManager &server, uint32_t id)
    {
        auto iter = std::ranges::find(_inits, id);
        if (iter == _inits.end())
            return;
        SetClientEntity::systemSetAllEntity<UpdateSprite2D,
            UpdateSpriteSheet2D,
            UpdateText2D>(gm, entity, server, *this, id);
        if (gm.hasComponent<Text2D>(entity)) {
            auto text = gm.getComponent<Text2D>(entity);
            TextProtocol::sendText({id}, server, _id, text.str);
        }
        _links.push_back(id);
        _inits.erase(iter);
    }

    void UpdateClientEntity::systemUpdateTextEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [_, text, update] :
            gm.view<UpdateText2D, UpdateClientEntity>().getVector()) {
            if (!text.getUpdateText() || !update.active || !update._update)
                continue;
            TextProtocol::sendText(update._links, server, update._id, text.str);
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

    void UpdateClientEntity::removeInit(uint32_t id)
    {
        for (auto iter = _inits.begin(); iter != _inits.end(); iter++) {
            if (*iter != id)
                continue;
            _inits.erase(iter);
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
            DestroyProtocol::sendDestroy(
                update.getLinks(), server, update.getId());
            gm.destroyEntity(e);
        } catch (NoSuchEntity &e) {
        }
    }

    void UpdateClientEntity::systemMasterUpdateClientEntitySleep(
        GameManager &gm, MasterClock &clock)
    {
        for (auto [_, update] : gm.view<UpdateClientEntity>().getVector()) {
            update._sleep += clock.getElapsed();
            update._update = update._sleep.count() > MILLI_IN_SEC / update.rate;
            if (update._update)
                update._sleep = std::chrono::milliseconds(0);
        }
    }
}; // namespace RType
