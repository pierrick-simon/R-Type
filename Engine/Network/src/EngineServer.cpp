/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineServer
*/

#include "EngineServer.hpp"

namespace RType {
    EngineServer::EngineServer(short port) :
        _udp(std::make_unique<UDPServer>(port))
    {
        _udp->start();
    }

    void EngineServer::systemServerReceived(
        EngineServer &server, GameManager &gm)
    {
        server._udp->received(gm);
    }
}; // namespace RType
