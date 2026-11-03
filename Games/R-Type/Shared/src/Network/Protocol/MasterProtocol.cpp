/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterProtocol
*/

#include "MasterProtocol.hpp"
#include <algorithm>
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
            [port](Entity, LobbyPortInfo &info) {
                if (port == 0)
                    info.refused = true;
                else
                    info.port = port;
            });
    }

    void MasterProtocol::manageJoinLobby(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() != 2)
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<LobbyRequest>(it->second))
            return;
        gm.getComponent<LobbyRequest>(it->second).joinPort =
            get16(payload.data());
    }

    void MasterProtocol::sendJoinLobby(ClientManager &c, uint16_t port)
    {
        std::vector<uint8_t> out(2);

        put16(out.data(), port);
        c.send(OpcodeRType::JOIN_LOBBY, out);
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

    void MasterProtocol::manageLobbyList(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        LobbyListState list;

        if (!ctx.gm || payload.empty())
            return;
        const std::size_t count = payload[0];

        if (payload.size() != 1 + count * 4)
            return;
        list.received = true;
        for (std::size_t i = 0; i < count; i++) {
            const uint8_t *p = &payload[1 + i * 4];

            list.lobbies.push_back(LobbyEntry {get16(p), p[2], p[3] != 0});
        }
        ctx.gm->get().view<LobbyListState>().each(
            [&list](Entity, LobbyListState &state) { state = list; });
    }

    std::vector<uint8_t> MasterProtocol::serializeLobbyList(
        const std::vector<LobbyEntry> &lobbies)
    {
        const std::size_t count = std::min<std::size_t>(lobbies.size(), 255);
        std::vector<uint8_t> out(1 + count * 4);

        out[0] = static_cast<uint8_t>(count);
        for (std::size_t i = 0; i < count; i++) {
            uint8_t *p = &out[1 + i * 4];

            put16(p, lobbies[i].port);
            p[2] = lobbies[i].players;
            p[3] = lobbies[i].started ? 1 : 0;
        }
        return out;
    }

} // namespace RType
