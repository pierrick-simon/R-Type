/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineClient
*/

#include "EngineClient.hpp"

namespace RType {
    EngineClient::EngineClient(const std::string &host, short port) :
        _udp(std::make_unique<UDPClient>(host, port))
    {
        _udp->start();
    }

    void EngineClient::systemClientReceived(
        EngineClient &client, GameManager &gm)
    {
        client._udp->received(gm);
    }
}; // namespace RType
