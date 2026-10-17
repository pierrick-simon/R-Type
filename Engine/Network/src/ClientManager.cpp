/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "ClientManager.hpp"
#include "Position2D.hpp"
#include "Sprite2D.hpp"
#include <iostream>

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
        _udp->addManage(
            SPRITE, [ctx = _ctx](const Header &h, const std::vector<uint8_t> &p) {
                manageSprite(h, p, ctx);
            });
    }

    void ClientManager::systemClientReceived(
        ClientManager &client, GameManager &gm)
    {
        client._ctx->gm = std::ref(gm);
        client._udp->received();
        client._ctx->gm.reset();
    }

    void ClientManager::manageSprite(
        const Header &header, const std::vector<uint8_t> &payload, std::shared_ptr<Context> ctx)
    {
        if (!ctx->gm) {
            return;
        }
        auto e = ctx->gm->get().createEntity();
        ctx->gm->get().addComponent<RType::Position2D>(e, 0, 0);
        ctx->gm->get().addComponent<RType::Texture2D>(e, "loading.png");
        ctx->gm->get().addComponent<RType::Sprite2D>(e);
        ctx->map.emplace(0, e);
    }
}; // namespace RType
