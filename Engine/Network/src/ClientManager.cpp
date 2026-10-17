/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "ClientManager.hpp"
#include "Position2D.hpp"
#include "Sprite2D.hpp"

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
            SPRITE, [this](const Header &h, const std::vector<uint8_t> &p) {
                manageSprite(h, p);
            });
    }

    void ClientManager::systemClientReceived(
        ClientManager &client, GameManager &gm)
    {
        client._gm = std::ref(gm);
        client._udp->received();
        client._gm.reset();
    }

    void ClientManager::manageSprite(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (!_gm)
            return;
        auto e = _gm->get().createEntity();
        _gm->get().addComponent<RType::Position2D>(e, 0, 0);
        _gm->get().addComponent<RType::Texture2D>(e, "loading.png");
        _gm->get().addComponent<RType::Sprite2D>(e);
    }
}; // namespace RType
