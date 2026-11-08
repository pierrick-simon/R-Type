/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AccountProtocol
*/

#include "AccountProtocol.hpp"
#include <algorithm>
#include "ServerManager.hpp"

namespace RType {

    bool AccountProtocol::isValidUsername(const std::string &username)
    {
        if (username.size() < ACCOUNT_USERNAME_MIN ||
            username.size() > ACCOUNT_USERNAME_MAX)
            return false;
        return std::all_of(username.begin(), username.end(), [](char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') || c == '_';
        });
    }

    bool AccountProtocol::isValidPassword(const std::string &password)
    {
        if (password.size() < ACCOUNT_PASSWORD_MIN ||
            password.size() > ACCOUNT_PASSWORD_MAX)
            return false;
        return std::all_of(password.begin(), password.end(), [](char c) {
            return c > ' ' && c <= '~';
        });
    }

    void AccountProtocol::sendRequest(ClientManager &c, AccountMode mode,
        const std::string &username, const std::string &password)
    {
        const std::string user = username.substr(0, ACCOUNT_USERNAME_MAX);
        const std::string pass = password.substr(0, ACCOUNT_PASSWORD_MAX);
        std::vector<uint8_t> out;

        out.push_back(static_cast<uint8_t>(mode));
        out.push_back(static_cast<uint8_t>(user.size()));
        out.insert(out.end(), user.begin(), user.end());
        out.push_back(static_cast<uint8_t>(pass.size()));
        out.insert(out.end(), pass.begin(), pass.end());
        c.send(OpcodeRType::ACCOUNT_REQUEST, out);
    }

    void AccountProtocol::manageRequest(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() < 3 ||
            payload[0] > static_cast<uint8_t>(AccountMode::Logout))
            return;
        const std::size_t userLength = payload[1];

        if (payload.size() < 3 + userLength)
            return;
        const std::size_t passLength = payload[2 + userLength];

        if (payload.size() != 3 + userLength + passLength)
            return;
        auto it = ctx.clients.find(header.senderId);
        if (it == ctx.clients.end())
            return;
        auto &gm = ctx.gm->get();
        if (!gm.hasComponent<AccountRequest>(it->second))
            return;
        auto &request = gm.getComponent<AccountRequest>(it->second);

        request.pending = true;
        request.mode = static_cast<AccountMode>(payload[0]);
        request.username = std::string(
            payload.begin() + 2, payload.begin() + long(2 + userLength));
        request.password =
            std::string(payload.begin() + long(3 + userLength), payload.end());
    }

    void AccountProtocol::sendResult(ServerManager &s, uint32_t clientId,
        AccountMode mode, AccountStatus status)
    {
        s.sendToClient(clientId,
            OpcodeRType::ACCOUNT_RESULT,
            {static_cast<uint8_t>(mode), static_cast<uint8_t>(status)});
    }

    void AccountProtocol::manageResult(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() != 2 ||
            payload[1] > static_cast<uint8_t>(AccountStatus::InvalidPassword))
            return;
        ctx.gm->get().view<AccountResult>().each(
            [&payload](Entity, AccountResult &result) {
                result.received = true;
                result.mode = static_cast<AccountMode>(payload[0]);
                result.status = static_cast<AccountStatus>(payload[1]);
            });
    }

} // namespace RType
