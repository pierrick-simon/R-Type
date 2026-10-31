/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyProtocol
*/

#include "LobbyProtocol.hpp"
#include <algorithm>
#include "ClientManager.hpp"

namespace RType {

    std::string LobbyProtocol::sanitizeName(const std::string &name)
    {
        std::string out;

        for (const char c : name) {
            if (out.size() >= LOBBY_NAME_MAX)
                break;
            if (c >= 32 && c <= 125)
                out += c;
        }
        return out;
    }

    void LobbyProtocol::manageInfo(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() < 2)
            return;
        const std::size_t len = payload[1];
        if (payload.size() != 2 + len)
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<LobbyMember>(it->second))
            return;
        auto &member = gm.getComponent<LobbyMember>(it->second);
        member.ready = payload[0] != 0;
        member.name = sanitizeName(std::string(payload.begin() + 2, payload.end()));
    }

    void LobbyProtocol::manageStart(
        const Header &header, const std::vector<uint8_t> &, Context &ctx)
    {
        if (!ctx.gm)
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<LobbyMember>(it->second))
            return;
        gm.getComponent<LobbyMember>(it->second).startRequested = true;
    }

    void LobbyProtocol::manageState(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm)
            return;
        auto state = parseState(payload);
        if (!state)
            return;
        ctx.gm->get().view<LobbyState>().each([&state](Entity, LobbyState &s) { s = *state; });
    }

    void LobbyProtocol::sendInfo(
        ClientManager &c, const std::string &name, bool ready)
    {
        const std::string clean = sanitizeName(name);
        std::vector<uint8_t> out(2 + clean.size());

        out[0] = ready ? 1 : 0;
        out[1] = static_cast<uint8_t>(clean.size());
        std::copy(clean.begin(), clean.end(), out.begin() + 2);
        c.send(OpcodeRType::LOBBY_INFO, out);
    }

    void LobbyProtocol::sendStart(ClientManager &c)
    {
        c.send(OpcodeRType::LOBBY_START, {});
    }

    std::vector<uint8_t> LobbyProtocol::serializeState(const LobbyState &s)
    {
        std::vector<uint8_t> out(4);
        const std::size_t count = std::min<std::size_t>(s.players.size(), 255);

        out[0] = s.started ? 1 : 0;
        put16(&out[1], s.hostId);
        out[3] = static_cast<uint8_t>(count);
        for (std::size_t i = 0; i < count; i++) {
            const auto &p = s.players[i];
            const std::string name = sanitizeName(p.name);
            const std::size_t at = out.size();

            out.resize(at + 4 + name.size());
            put16(&out[at], p.id);
            out[at + 2] = p.ready ? 1 : 0;
            out[at + 3] = static_cast<uint8_t>(name.size());
            std::copy(name.begin(), name.end(), out.begin() + long(at + 4));
        }
        return out;
    }

    std::optional<LobbyState> LobbyProtocol::parseState(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 4)
            return std::nullopt;
        LobbyState s;
        std::size_t at = 4;
        const std::size_t count = payload[3];

        s.received = true;
        s.started = payload[0] != 0;
        s.hostId = get16(&payload[1]);
        for (std::size_t i = 0; i < count; i++) {
            if (payload.size() < at + 4)
                return std::nullopt;
            LobbyPlayerInfo p;
            const std::size_t len = payload[at + 3];

            p.id = get16(&payload[at]);
            p.ready = payload[at + 2] != 0;
            if (payload.size() < at + 4 + len)
                return std::nullopt;
            p.name.assign(payload.begin() + long(at + 4),
                payload.begin() + long(at + 4 + len));
            at += 4 + len;
            s.players.push_back(std::move(p));
        }
        if (at != payload.size())
            return std::nullopt;
        return s;
    }

} // namespace RType
