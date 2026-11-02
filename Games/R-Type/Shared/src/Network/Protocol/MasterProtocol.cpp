/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterProtocol
*/

#include "MasterProtocol.hpp"
#include "ServerManager.hpp"

namespace RType {

    void MasterProtocol::manageCreateLobby(
        const Header &header, const std::vector<uint8_t> &, Context &ctx)
    {
        if (!ctx.gm)
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<LobbyRequest>(it->second))
            return;
        gm.getComponent<LobbyRequest>(it->second).createRequested = true;
    }

    void MasterProtocol::manageLobbyPort(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() != 2)
            return;
        const uint16_t port = get16(payload.data());

        ctx.gm->get().view<LobbyPortInfo>().each(
            [port](Entity, LobbyPortInfo &info) { info.port = port; });
    }

    void MasterProtocol::sendCreateLobby(ClientManager &c)
    {
        c.send(OpcodeRType::CREATE_LOBBY, {});
    }

    void MasterProtocol::sendLobbyPort(
        ServerManager &s, uint32_t clientId, uint16_t port)
    {
        std::vector<uint8_t> out(2);

        put16(out.data(), port);
        s.sendToClient(clientId, OpcodeRType::LOBBY_PORT, out);
    }

} // namespace RType
