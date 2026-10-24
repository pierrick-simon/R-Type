/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateClientEntity
*/

#include "UpdateClientEntity.hpp"
#include <unordered_set>
#include "CreateProtocol.hpp"
#include "DestroyProtocol.hpp"
#include "MoveProtocol.hpp"
#include "SetClientEntity.hpp"
#include "SetProtocol.hpp"
#include "TextProtocol.hpp"

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
            gm.view<Position2D, UpdateClientEntity>().getVector()) {
            if (!update.active || !update._update)
                continue;
            if (!gm.hasComponent<Velocity2D>(entity))
                MoveProtocol::sendMove(
                    update._links, server, update.getId(), pos);
            else {
                auto &v = gm.getComponent<Velocity2D>(entity);
                MoveProtocol::sendMove(
                    update._links, server, update.getId(), pos, v);
            }
        }
    }

    void UpdateClientEntity::systemCreateClientEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [entity, update] :
            gm.view<UpdateClientEntity>().getVector()) {
            ProtocolEntity pe = {};
            removeCommon(update.create, update._links);
            if (!update.active || !update._update || update.create.empty())
                continue;
            checkConnectedClient(server, update);
            try {
                auto tmp = gm.getComponent<Position2D>(entity);
                pe.posX = uint32_t(tmp.x);
                pe.posY = uint32_t(tmp.y);
            } catch (NoSuchComponent &e) {
            }
            pe.assetId = update.asset;
            pe.id = update.getId();
            pe.type = update._type;
            CreateProtocol::sendCreate(update.create, server, pe);
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
        try {
            auto text = gm.getComponent<Text2D>(entity);
            TextProtocol::sendText(update.create, server, update._id, text.str);
        } catch (NoSuchComponent &e) {
        }
    }

    void UpdateClientEntity::systemUpdateTextEntity(
        GameManager &gm, ServerManager &server)
    {
        for (auto [_, text, update] :
            gm.view<Text2D, UpdateClientEntity>().getVector()) {
            if (!update.active || !update._update)
                continue;
            TextProtocol::sendText(update._links, server, update._id, text.str);
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
