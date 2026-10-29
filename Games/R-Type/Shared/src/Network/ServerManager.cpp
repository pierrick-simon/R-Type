/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#include "ServerManager.hpp"
#include "CreatedProtocol.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"
#include "UpdateClientEntity.hpp"
#include "DestroyProtocol.hpp"

namespace RType {
    ServerManager::ServerManager(short port) :
        _udp(std::make_unique<UDPServer>(port))
    {
    }

    void ServerManager::createProtocol()
    {
        _udp->addManage(
            LOGIN,
            [this]([[maybe_unused]] const Header &h,
                [[maybe_unused]] const std::vector<uint8_t> &p) {
                defaultClient();
            },
            true);
        _udp->addManage(
            LOGOUT,
            [this](const Header &h,
                [[maybe_unused]] const std::vector<uint8_t> &p) {
                deleteClient(h);
            },
            true);
        _udp->onClientRemoved = [this](const Header &h) { deleteClient(h); };
        _udp->addManage(
            CREATE, [this](const Header &h, const std::vector<uint8_t> &p) {
                CreatedProtocol::manageCreated(h, p, *this);
            });
        addContextManage(KEY_PRESSED, KeyboardProtocol::manageKeyPressed);
        addContextManage(KEY_RELEASED, KeyboardProtocol::manageKeyReleased);
        addContextManage(
            MOUSE_BUTTON_PRESSED, MouseProtocol::manageButtonPressed);
        addContextManage(
            MOUSE_BUTTON_RELEASED, MouseProtocol::manageButtonReleased);
    }

    void ServerManager::systemServerReceived(
        GameManager &gm, ServerManager &server)
    {
        if (server._first) {
            server._first = false;
            server.createProtocol();
            server._udp->start();
        }
        server._ctx.gm = std::ref(gm);
        server._udp->received();
        server._ctx.gm.reset();
    }

    void ServerManager::addContextManage(uint8_t op,
        const std::function<void(
            const Header &, const std::vector<uint8_t> &, Context &)> &func)
    {
        _udp->addManage(
            op, [this, func](const Header &h, const std::vector<uint8_t> &p) {
                func(h, p, _ctx);
            });
    }

    void ServerManager::sendToClients(const std::vector<uint32_t> &ids,
        uint8_t op, const std::vector<uint8_t> &payload)
    {
        for (auto id : ids)
            sendToClient(id, op, payload);
    }

    void ServerManager::sendToClient(
        uint32_t id, uint8_t op, const std::vector<uint8_t> &payload)
    {
        _udp->sendToClient(id, op, payload);
    }

    void ServerManager::broadcast(
        uint8_t op, const std::vector<uint8_t> &payload)
    {
        for (auto [id, _] : _ctx.clients)
            sendToClient(id, op, payload);
    }

    void ServerManager::defaultClient()
    {
        if (!_ctx.gm)
            return;
        auto id = _udp->addNewClient();
        auto entity = _ctx.gm->get().createEntity();
        _ctx.clients.emplace(id, entity);
        _ctx.gm->get().addComponent<DefaulClient>(entity, id);
        _ctx.gm->get().addComponent<KeyboardManager>(entity);
        _ctx.gm->get().addComponent<MouseManager>(entity);
        for (auto [_, update] :
            _ctx.gm->get().view<UpdateClientEntity>().getVector())
            update.create.push_back(id);
    }

    void ServerManager::deleteClient(const Header &h)
    {
        if (!_ctx.gm)
            return;
        _udp->removeClient(h);
        if (_ctx.clients.count(h.senderId) == 0)
            return;
        auto &gm = _ctx.gm->get();

        for (auto [_, update] : gm.view<UpdateClientEntity>().getVector()) {
            auto &l = update.getLinks();
            l.erase(std::remove_if(l.begin(),
                        l.end(),
                        [&](uint32_t v) { return h.senderId == v; }),
                l.end());
            auto &i = update.getInits();
            i.erase(std::remove_if(i.begin(),
                        i.end(),
                        [&](uint32_t v) { return h.senderId == v; }),
                i.end());
            auto &c = update.create;
            c.erase(std::remove_if(c.begin(),
                        c.end(),
                        [&](uint32_t v) { return h.senderId == v; }),
                c.end());
        }
        const Entity player = _ctx.clients.at(h.senderId);

        if (gm.hasComponent<UpdateClientEntity>(player)) {
            auto &update = gm.getComponent<UpdateClientEntity>(player);
            std::vector<uint32_t> ids = update.getLinks();

            ids.insert(
                ids.end(), update.getInits().begin(), update.getInits().end());
            DestroyProtocol::sendDestroy(ids, *this, update.getId());
        }
        gm.destroyEntity(player);
        _ctx.clients.erase(h.senderId);
    }
}; // namespace RType
