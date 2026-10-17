/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#include "ServerManager.hpp"

namespace RType {
    ServerManager::ServerManager(short port, bool defaultProtocole) :
        _udp(std::make_unique<UDPServer>(port)),
        _defaultProtocole(defaultProtocole)
    {
    }

    void ServerManager::createDefaultProtocole()
    {
    }

    void ServerManager::systemServerReceived(
        ServerManager &server, GameManager &gm)
    {
        if (server._first) {
            server._first = false;
            if (server._defaultProtocole)
                server.createDefaultProtocole();
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
}; // namespace RType
