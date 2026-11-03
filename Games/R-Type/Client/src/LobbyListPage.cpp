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
#include "MouseManager.hpp"
#include "KeyboardManager.hpp"

namespace RType::Client {

    namespace {

        void requestLobby(GameManager &gm)
        {
            gm.view<LobbyListClient>().each([](Entity, LobbyListClient &l) {
                l.createRequested = true;
                l.lastRequest = {};
            });
        }

        void requestJoin(GameManager &gm)
        {
            gm.view<LobbyListClient>().each([](Entity, LobbyListClient &l) {
                l.joinPort = l.selectedPort;
                l.lastRequest = {};
            });
        }

        void leaveServer(GameManager &gm)
        {
            std::vector<Entity> toDestroy;

            gm.view<ClientManager>().each(
                [&toDestroy](Entity e, ClientManager &) { toDestroy.push_back(e); });
            gm.view<LobbyListItem>().each(
                [&toDestroy](Entity e, LobbyListItem &) { toDestroy.push_back(e); });
            for (const Entity e : toDestroy)
                gm.destroyEntity(e);
            createServerPage(gm);
        }

        void switchToLobby(GameManager &gm, Entity master, uint16_t port)
        {
            destroyLobbyListPage(gm);
            gm.destroyEntity(master);
            connectToServer(gm, _clInfos._ip, port);
            showConnecting(gm);
        }

        Rectangle rowRect(const std::size_t row)
        {
            return Rectangle {
                (static_cast<float>(WINDOW_WIDTH) - LOBBY_LIST_ROW_WIDTH) / 2.f,
                LOBBY_LIST_FIRST_ROW_Y
                    + static_cast<float>(row)
                        * (LOBBY_LIST_ROW_HEIGHT + LOBBY_LIST_ROW_GAP) + scroll,
                LOBBY_LIST_ROW_WIDTH,
                LOBBY_LIST_ROW_HEIGHT};
        }

        bool inside(const Rectangle &rect, const Vector2 &point)
        {
            return point.x >= rect.x && point.x <= rect.x + rect.width
                && point.y >= rect.y && point.y <= rect.y + rect.height;
        }

        bool leftClicked(GameManager &gm)
        {
            bool pressed = false;

            gm.view<MouseManager>().each([&pressed](Entity, MouseManager &m) {
                for (const auto button : m.buttonPressed)
                    if (button == MouseButton::Value::Left)
                        pressed = true;
            });
            return pressed;
        }

        void selectClickedLobby(
            GameManager &gm, LobbyListClient &l, const LobbyListState &list)
        {
            if (!leftClicked(gm))
                return;
            const Vector2 mouse = ::GetMousePosition();

            for (std::size_t i = 0;
                i < list.lobbies.size() && i < LOBBY_LIST_MAX_ROWS;
                i++)
                if (inside(rowRect(i), mouse)) {
                    if (l.selectedPort == list.lobbies[i].port)
                        l.selectedPort = 0;
                    else
                        l.selectedPort = list.lobbies[i].port;
                }
        }

        const LobbyEntry *selectedLobby(
            const LobbyListClient &l, const LobbyListState &list)
        {
            for (const auto &lobby : list.lobbies)
                if (lobby.port == l.selectedPort)
                    return &lobby;
            return nullptr;
        }

    } // namespace

    void connectToMaster(GameManager &gm, const std::string &host, int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, host, static_cast<short>(port));
        gm.addComponent<LobbyListClient>(c);
        gm.addComponent<LobbyPortInfo>(c);
        gm.addComponent<LobbyListState>(c);
    }

    void createLobbyListPage(GameManager &gm)
    {
        const float buttonsY =
            static_cast<float>(WINDOW_HEIGTH) - BUTTON_HEIGHT * 2 - BUTTON_GAP;
        const Entity title = addText(gm,
            static_cast<float>(WINDOW_WIDTH) / 2.f,
            LOBBY_LIST_TITLE_Y,
            "Lobbies",
            LOBBY_LIST_TITLE_SIZE);
        const Entity create = addButton(
            gm, BUTTON_GAP * 2, buttonsY, "Create Lobby", requestLobby);
        const Entity join = addButton(gm,
            (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f,
            buttonsY,
            "Join Lobby",
            requestJoin);
        const Entity leave = addButton(gm,
            static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH - BUTTON_GAP * 2,
            buttonsY,
            "Leave Server",
            leaveServer);

        gm.addComponent<LobbyListItem>(title);
        gm.addComponent<LobbyListItem>(create);
        gm.addComponent<CreateLobbyButton>(create);
        gm.addComponent<LobbyListItem>(join);
        gm.addComponent<JoinLobbyButton>(join);
        gm.addComponent<LobbyListItem>(leave);
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
        bool requesting = false;
        bool canJoin = false;
        std::optional<std::pair<Entity, uint16_t>> target;

        gm.view<ClientManager, LobbyListClient, LobbyPortInfo, LobbyListState>()
            .each([&](Entity e,
                      ClientManager &c,
                      LobbyListClient &l,
                      LobbyPortInfo &info,
                      LobbyListState &list) {
                const auto now = std::chrono::steady_clock::now();

                if (!c.isConnected())
                    return;
                if (info.port != 0) {
                    target = {e, info.port};
                    return;
                }
                if (info.refused) {
                    info.refused = false;
                    l.joinPort = 0;
                }
                if (!l.shown) {
                    l.shown = true;
                    show = true;
                }
                selectClickedLobby(gm, l, list);
                const LobbyEntry *selected = selectedLobby(l, list);

                if (selected == nullptr)
                    l.selectedPort = 0;
                if (now - l.lastRequest >= LOBBY_LIST_RESEND_RATE) {
                    if (l.createRequested)
                        MasterProtocol::sendCreateLobby(c);
                    else if (l.joinPort != 0)
                        MasterProtocol::sendJoinLobby(c, l.joinPort);
                    l.lastRequest = now;
                }
                requesting = l.createRequested || l.joinPort != 0;
                canJoin = selected != nullptr && !requesting
                    && selected->players < MAX_LOBBY_PLAYERS;
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
        gm.view<CreateLobbyButton, Button>().each(
            [requesting](Entity, CreateLobbyButton &, Button &button) {
                button.disabled = requesting;
            });
        gm.view<JoinLobbyButton, Button>().each(
            [canJoin](Entity, JoinLobbyButton &, Button &button) {
                button.disabled = !canJoin;
            });
        if (target)
            switchToLobby(gm, target->first, target->second);
    }

    void systemDrawLobbyList(GameManager &gm)
    {
        const LobbyListState *state = nullptr;
        const LobbyListClient *client = nullptr;
        const Font *font = nullptr;

        gm.view<LobbyListState, LobbyListClient>().each(
            [&](Entity, LobbyListState &s, LobbyListClient &l) {
                state = &s;
                client = &l;
            });
        gm.view<LobbyListItem, Font>().each(
            [&font](Entity, LobbyListItem &, Font &f) { font = &f; });
        if (state == nullptr || font == nullptr)
            return;
        const float size = LOBBY_LIST_TEXT_SIZE;
        auto text = [&](const std::string &str, float tx, float ty, Color color) {
            ::DrawTextEx(*font, str.c_str(), {tx, ty}, size, 1.f, color);
        };
        auto width = [&](const std::string &str) {
            return ::MeasureTextEx(*font, str.c_str(), size, 1.f).x;
        };
        if (state->lobbies.size() <= LOBBY_LIST_MAX_ROWS) {
            scroll = 0;
        } else {
            gm.view<KeyboardManager>().each([&state](Entity, KeyboardManager &m) {
                for (const auto key : m.keyDown)
                    if (key == Key::Value::Down)
                        scroll -= 5;
                    else if (key == Key::Value::Up)
                        scroll += 5;
                if (scroll > 0)
                    scroll = 0;
                if (scroll < (state->lobbies.size() - LOBBY_LIST_MAX_ROWS) * -(LOBBY_LIST_ROW_HEIGHT + LOBBY_LIST_ROW_GAP))
                    scroll = (state->lobbies.size() - LOBBY_LIST_MAX_ROWS) * -(LOBBY_LIST_ROW_HEIGHT + LOBBY_LIST_ROW_GAP);
            });
        }
        if (!state->lobbies.empty()) 
            for (std::size_t i = 0; i < state->lobbies.size(); i++) {
                const LobbyEntry &lobby = state->lobbies[i];
                const Rectangle rect = rowRect(i);
                const bool selected = lobby.port == client->selectedPort;
                const float textY = rect.y + (LOBBY_LIST_ROW_HEIGHT - size) / 2.f;
                const std::string name = "Lobby " + std::to_string(i + 1);
                const std::string players = std::to_string(lobby.players) + " / "
                    + std::to_string(MAX_LOBBY_PLAYERS) + " players";
                const std::string status = lobby.started ? "In game" : "Waiting";

                ::DrawRectangleRec(rect,
                    selected ? Color {60, 110, 200, 240} : Color {35, 42, 75, 230});
                ::DrawRectangleLinesEx(rect, selected ? 5.f : 3.f,
                    selected ? Color {255, 210, 80, 255} : Color {200, 200, 220, 255});
                text(name, rect.x + 30.f, textY, WHITE);
                text(players, rect.x + LOBBY_LIST_PLAYERS_X, textY, WHITE);
                text(status, rect.x + LOBBY_LIST_ROW_WIDTH - 30.f - width(status), textY,
                    lobby.started ? Color {255, 170, 60, 255} : Color {90, 230, 120, 255});
            }
        ::DrawRectangleRec(Rectangle{0, 0, 1920, 180}, Color {60, 60, 100, 255});
        ::DrawRectangleRec(Rectangle{0, 830, 1920, 200}, Color {60, 60, 100, 255});
        if (state->lobbies.empty()) {
            const std::string empty = state->received
                ? "No lobby yet, create one"
                : "Loading lobbies...";

            text(empty, (static_cast<float>(WINDOW_WIDTH) - width(empty)) / 2.f,
                LOBBY_LIST_FIRST_ROW_Y + 20.f, Color {200, 205, 225, 255});
            return;
        }
    }

} // namespace RType::Client
