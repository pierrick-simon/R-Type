/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "ClientManager.hpp"

namespace RType {
    ClientManager::ClientManager(
        const std::string &host, short port, bool defaultProtocole) :
        _udp(std::make_unique<UDPClient>(host, port))
    {
        if (defaultProtocole)
            createDefaultProtocol();
        _udp->start();
    }

    void ClientManager::createDefaultProtocol()
    {
    }

    void ClientManager::systemClientReceived(
        ClientManager &client, GameManager &gm)
    {
        client._ctx->gm = std::ref(gm);
        client._udp->received();
        client._ctx->gm.reset();
    }

    void ClientManager::send(const Opcode op, const std::vector<uint8_t> &payload)
    {
        _udp->send(op, payload);
    }

    void ClientManager::addContextManage(uint8_t op,
        const std::function<void(const Header &, const std::vector<uint8_t> &,
            std::shared_ptr<Context>)> &func)
    {
        _udp->addManage(op,
            [ctx = _ctx, func](const Header &h, const std::vector<uint8_t> &p) {
                func(h, p, ctx);
            });
    }
}; // namespace RType
