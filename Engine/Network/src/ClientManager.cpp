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
        _udp(std::make_unique<UDPClient>(host, port)),
        _defaultProtocole(defaultProtocole)
    {
    }

    void ClientManager::createDefaultProtocol()
    {
    }

    void ClientManager::systemClientReceived(
        GameManager &gm, ClientManager &client)
    {
        if (client._first) {
            client._first = false;
            if (client._defaultProtocole)
                client.createDefaultProtocol();
            client._udp->start();
        }
        client._ctx.gm = std::ref(gm);
        client._udp->received();
        client._ctx.gm.reset();
    }

    void ClientManager::send(Opcode op, const std::vector<uint8_t> &payload)
    {
        _udp->send(op, payload);
    }

    void ClientManager::addContextManage(Opcode op,
        const std::function<void(
            const Header &, const std::vector<uint8_t> &, Context &)> &func)
    {
        _udp->addManage(
            op, [this, func](const Header &h, const std::vector<uint8_t> &p) {
                func(h, p, _ctx);
            });
    }
}; // namespace RType
