/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyListPage
*/

#include "LobbyListPage.hpp"

#include <optional>
#include <vector>

#include "Client.hpp"
#include "ClientManager.hpp"
#include "MasterProtocol.hpp"
#include "Menu.hpp"

namespace RType::Client {

    namespace {

        void requestLobby(GameManager &gm)
        {
            gm.view<LobbyListClient>().each([](Entity, LobbyListClient &l) {
                l.createRequested = true;
                l.lastRequest = {};
            });
        }

        void switchToLobby(GameManager &gm, Entity master, uint16_t port)
        {
            destroyLobbyListPage(gm);
            gm.destroyEntity(master);
            connectToServer(gm, _clInfos._ip, port);
            showConnecting(gm);
        }

    } // namespace

    void connectToMaster(GameManager &gm, const std::string &host, int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, host, static_cast<short>(port));
        gm.addComponent<LobbyListClient>(c);
        gm.addComponent<LobbyPortInfo>(c);
    }

    void createLobbyListPage(GameManager &gm)
    {
        const Entity button = addButton(gm,
            (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f,
            (static_cast<float>(WINDOW_HEIGTH) - BUTTON_HEIGHT) / 2.f,
            "Create Lobby",
            requestLobby);

        gm.addComponent<LobbyListItem>(button);
    }

    void destroyLobbyListPage(GameManager &gm)
    {
        std::vector<Entity> items;

        gm.view<LobbyListItem>().each(
            [&items](Entity e, LobbyListItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
    }

    void systemLobbyListPage(GameManager &gm)
    {
        bool show = false;
        bool requested = false;
        std::optional<std::pair<Entity, uint16_t>> target;

        gm.view<ClientManager, LobbyListClient, LobbyPortInfo>().each(
            [&](Entity e,
                ClientManager &c,
                LobbyListClient &l,
                LobbyPortInfo &info) {
                const auto now = std::chrono::steady_clock::now();

                if (!c.isConnected())
                    return;
                if (info.port != 0) {
                    target = {e, info.port};
                    return;
                }
                if (!l.shown) {
                    l.shown = true;
                    show = true;
                }
                if (l.createRequested &&
                    now - l.lastRequest >= LOBBY_LIST_RESEND_RATE) {
                    MasterProtocol::sendCreateLobby(c);
                    l.lastRequest = now;
                }
                requested = l.createRequested;
            });
        if (show) {
            std::vector<Entity> connecting;

            gm.view<ConnectingText>().each(
                [&connecting](
                    Entity e, ConnectingText &) { connecting.push_back(e); });
            for (const Entity e : connecting)
                gm.destroyEntity(e);
            createLobbyListPage(gm);
        }
        gm.view<LobbyListItem, Button>().each(
            [requested](Entity, LobbyListItem &, Button &button) {
                button.disabled = requested;
            });
        if (target)
            switchToLobby(gm, target->first, target->second);
    }

} // namespace RType::Client
