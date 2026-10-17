/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#include "ServerManager.hpp"

namespace RType {
    ServerManager::ServerManager(short port) :
        _udp(std::make_unique<UDPServer>(port))
    {
        _udp->start();
    }

    void ServerManager::systemServerReceived(
        ServerManager &server, GameManager &gm)
    {
        server._udp->received(gm);
    }
}; // namespace RType
