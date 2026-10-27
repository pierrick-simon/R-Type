/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "ClientManager.hpp"
#include "CreateProtocol.hpp"
#include "CreatedProtocol.hpp"
#include "DestroyProtocol.hpp"
#include "MoveProtocol.hpp"
#include "SetProtocol.hpp"
#include "SoundProtocol.hpp"
#include "TextProtocol.hpp"

namespace RType {
    ClientManager::ClientManager(const std::string &host, short port) :
        _udp(std::make_unique<UDPClient>(host, port))
    {
    }

    void ClientManager::createProtocol()
    {
        _udp->addManage(
            CREATE, [this](const Header &h, const std::vector<uint8_t> &p) {
                CreateProtocol::manageCreate(h, p, *this);
            });
        addContextManage(OpcodeRType::DESTROY, DestroyProtocol::manageDestroy);
        addContextManage(OpcodeRType::MOVE, MoveProtocol::manageMove);
        addContextManage(OpcodeRType::SET, SetProtocol::manageSet);
        addContextManage(OpcodeRType::PAUSE, SoundProtocol::managePause);
        addContextManage(OpcodeRType::PLAY, SoundProtocol::managePlay);
        addContextManage(OpcodeRType::STOP, SoundProtocol::manageStop);
        addContextManage(OpcodeRType::TEXT, TextProtocol::manageText);
    }

    void ClientManager::systemClientReceived(
        GameManager &gm, ClientManager &client)
    {
        if (client._first) {
            client._first = false;
            client.createProtocol();
            client._udp->start();
        }
        client._ctx.gm = std::ref(gm);
        client._udp->received();
        client._ctx.gm.reset();
    }

    void ClientManager::send(uint8_t op, const std::vector<uint8_t> &payload)
    {
        _udp->send(op, payload);
    }

    void ClientManager::addContextManage(uint8_t op,
        const std::function<void(
            const Header &, const std::vector<uint8_t> &, Context &)> &func)
    {
        _udp->addManage(
            op, [this, func](const Header &h, const std::vector<uint8_t> &p) {
                func(h, p, _ctx);
            });
    }

    void ClientManager::systemEntityCreated(ClientManager &c)
    {
        for (auto id : c._created) {
            CreatedProtocol::sendCreated(c, id);
        }
        c._created.clear();
    }

    void ClientManager::addCreatedEntity(uint32_t id)
    {
        if (std::ranges::find(_created, id) == _created.end())
            _created.push_back(id);
    }
}; // namespace RType
