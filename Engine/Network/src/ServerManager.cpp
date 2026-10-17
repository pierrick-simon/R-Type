/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#include "ServerManager.hpp"

namespace RType {
    ServerManager::ServerManager(short port, bool defaultProtocole) :
        _udp(std::make_unique<UDPServer>(port))
    {
        if (defaultProtocole)
            createDefaultProtocol();
        _udp->start();
    }

    void ServerManager::createDefaultProtocol()
    {
    }

    void ServerManager::systemServerReceived(
        ServerManager &server, GameManager &gm)
    {
        server._ctx->gm = std::ref(gm);
        server._udp->received();
        server._ctx->gm.reset();
    }

    void ServerManager::addContextManage(uint8_t op,
        const std::function<void(const Header &, const std::vector<uint8_t> &,
            std::shared_ptr<Context>)> &func)
    {
        _udp->addManage(op,
            [ctx = _ctx, func](const Header &h, const std::vector<uint8_t> &p) {
                func(h, p, ctx);
            });
    }
}; // namespace RType
