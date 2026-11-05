/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterProtocol
*/

#include "MasterProtocol.hpp"
#include <algorithm>
#include "LobbyProtocol.hpp"
#include "ServerManager.hpp"

namespace RType {

    void MasterProtocol::manageCreateLobby(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty() || payload.size() != 1u + payload[0])
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<LobbyRequest>(it->second))
            return;
        auto &request = gm.getComponent<LobbyRequest>(it->second);

        request.createRequested = true;
        request.name = LobbyProtocol::sanitizeName(
            std::string(payload.begin() + 1, payload.end()));
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

    void MasterProtocol::sendCreateLobby(
        ClientManager &c, const std::string &name)
    {
        const std::string clean = LobbyProtocol::sanitizeName(name);
        std::vector<uint8_t> out(1 + clean.size());

        out[0] = static_cast<uint8_t>(clean.size());
        std::copy(clean.begin(), clean.end(), out.begin() + 1);
        c.send(OpcodeRType::CREATE_LOBBY, out);
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
        std::size_t at = 1;

        for (std::size_t i = 0; i < count; i++) {
            if (payload.size() < at + 5)
                return;
            const uint8_t *p = &payload[at];
            const std::size_t length = p[4];

            if (payload.size() < at + 5 + length)
                return;
            list.lobbies.push_back(LobbyEntry {get16(p),
                p[2],
                p[3] != 0,
                std::string(payload.begin() + long(at + 5),
                    payload.begin() + long(at + 5 + length))});
            at += 5 + length;
        }
        if (at != payload.size())
            return;
        list.received = true;
        ctx.gm->get().view<LobbyListState>().each(
            [&list](Entity, LobbyListState &state) { state = list; });
    }

    std::vector<uint8_t> MasterProtocol::serializeLobbyList(
        const std::vector<LobbyEntry> &lobbies)
    {
        const std::size_t count =
            std::min<std::size_t>(lobbies.size(), MAX_LISTED_LOBBIES);
        std::vector<uint8_t> out(1);

        out[0] = static_cast<uint8_t>(count);
        for (std::size_t i = 0; i < count; i++) {
            const std::string name =
                LobbyProtocol::sanitizeName(lobbies[i].name);
            const std::size_t at = out.size();

            out.resize(at + 5 + name.size());
            put16(&out[at], lobbies[i].port);
            out[at + 2] = lobbies[i].players;
            out[at + 3] = lobbies[i].started ? 1 : 0;
            out[at + 4] = static_cast<uint8_t>(name.size());
            std::copy(name.begin(), name.end(), out.begin() + long(at + 5));
        }
        return out;
    }

} // namespace RType
