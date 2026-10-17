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
            createDefaultProtocole();
        _udp->start();
    }

    void ServerManager::createDefaultProtocole()
    {
    }

    void ServerManager::systemServerReceived(
        ServerManager &server, GameManager &gm)
    {
        server._ctx->gm = std::ref(gm);
        server._udp->received();
        server._ctx->gm.reset();
    }
}; // namespace RType
