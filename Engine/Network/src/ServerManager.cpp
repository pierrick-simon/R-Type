/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#include "ServerManager.hpp"
#include "KeyboardManager.hpp"
#include "KeyboardProtocol.hpp"
#include "MouseManager.hpp"
#include "MouseProtocol.hpp"

namespace RType {
    ServerManager::ServerManager(short port, bool defaultProtocole) :
        _udp(std::make_unique<UDPServer>(port)),
        _defaultProtocole(defaultProtocole)
    {
    }

    void ServerManager::createDefaultProtocol()
    {
        _udp->addManage(
            LOGIN,
            [this](const Header &h, const std::vector<uint8_t> &p) {
                defaultClient();
            },
            true);
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
            if (server._defaultProtocole)
                server.createDefaultProtocol();
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
    }
}; // namespace RType
