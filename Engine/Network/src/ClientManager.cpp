/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "ClientManager.hpp"

namespace RType {
    ClientManager::ClientManager(const std::string &host, short port) :
        _udp(std::make_unique<UDPClient>(host, port))
    {
        _udp->start();
    }

    void ClientManager::systemClientReceived(
        ClientManager &client, GameManager &gm)
    {
        client._udp->received(gm);
    }
}; // namespace RType
