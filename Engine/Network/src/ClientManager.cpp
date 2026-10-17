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
            createDefaultProtocole();
        _udp->start();
    }

    void ClientManager::createDefaultProtocole()
    {
    }

    void ClientManager::systemClientReceived(
        ClientManager &client, GameManager &gm)
    {
        client._ctx->gm = std::ref(gm);
        client._udp->received();
        client._ctx->gm.reset();
    }
}; // namespace RType
