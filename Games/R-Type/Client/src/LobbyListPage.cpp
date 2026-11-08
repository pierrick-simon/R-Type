/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** LobbyListPage
*/

#include <algorithm>
#include <optional>
#include <set>
#include <vector>

#include "Client.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "KeyboardManager.hpp"
#include "LobbyListPage.hpp"
#include "Login.hpp"
#include "MasterProtocol.hpp"
#include "Menu.hpp"
#include "MouseManager.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        template<typename Tag>
        void destroyTagged(GameManager &gm)
        {
            std::vector<Entity> items;

            gm.view<Tag>().each(
                [&items](Entity e, Tag &) { items.push_back(e); });
            for (const Entity e : items)
                gm.destroyEntity(e);
        }

        LobbyListClient *findClient(GameManager &gm)
        {
            LobbyListClient *client = nullptr;

            gm.view<LobbyListClient>().each(
                [&client](Entity, LobbyListClient &l) { client = &l; });
            return client;
        }

        std::string defaultLobbyName(const LobbyListState &list)
        {
            const std::string prefix = "Lobby ";
            std::set<int> used;
            int n = 1;

            for (const auto &lobby : list.lobbies) {
                const std::string &name = lobby.name;

                if (name.size() <= prefix.size() ||
                    name.size() > prefix.size() + 6 ||
                    name.compare(0, prefix.size(), prefix) != 0)
                    continue;
                const std::string digits = name.substr(prefix.size());

                if (std::all_of(digits.begin(), digits.end(), [](char c) {
                        return c >= '0' && c <= '9';
                    }))
                    used.insert(std::stoi(digits));
            }
            while (used.contains(n))
                n++;
            return prefix + std::to_string(n);
        }

        void closeCreatePopup(GameManager &gm)
        {
            LobbyListClient *l = findClient(gm);

            if (l == nullptr || l->createRequested)
                return;
            destroyTagged<CreatePopupItem>(gm);
            l->popupOpen = false;
            if (activeField == Field::LobbyName)
                activeField = Field::None;
        }

        void confirmCreate(GameManager &gm)
        {
            LobbyListClient *l = findClient(gm);

            if (l == nullptr || l->createRequested)
                return;
            if (_clInfos._lobbyName.empty())
                _clInfos._lobbyName = _clInfos._lobbyNameDefault;
            l->createName = _clInfos._lobbyName;
            l->createRequested = true;
            l->lastRequest = {};
            if (activeField == Field::LobbyName)
                activeField = Field::None;
        }

        Entity addPopupButton(GameManager &gm, float x, float y,
            const std::string &label,
            std::function<void(GameManager &)> onClick)
        {
            const Entity e = addButton(gm, x, y, label, std::move(onClick));

            gm.addComponent<LobbyListItem>(e);
            gm.addComponent<CreatePopupItem>(e);
            return e;
        }

        void addPopupText(GameManager &gm, float y, const std::string &str,
            float size, raylib::Color color = raylib::Color::White())
        {
            const Entity e = addText(gm,
                static_cast<float>(WINDOW_WIDTH) / 2.f,
                y,
                str,
                size,
                {},
                color);

            gm.addComponent<LobbyListItem>(e);
            gm.addComponent<CreatePopupItem>(e);
        }

        void openCreatePopup(GameManager &gm)
        {
            LobbyListClient *l = nullptr;
            const LobbyListState *list = nullptr;

            gm.view<LobbyListClient, LobbyListState>().each(
                [&](Entity, LobbyListClient &c, LobbyListState &s) {
                    l = &c;
                    list = &s;
                });
            if (l == nullptr || l->popupOpen)
                return;
            const float x =
                (static_cast<float>(WINDOW_WIDTH) - CREATE_POPUP_WIDTH) / 2.f;
            const float y =
                (static_cast<float>(WINDOW_HEIGTH) - CREATE_POPUP_HEIGHT) / 2.f;
            const float buttonsY =
                y + CREATE_POPUP_HEIGHT - CREATE_POPUP_PADDING - BUTTON_HEIGHT;

            _clInfos._lobbyNameDefault = defaultLobbyName(*list);
            _clInfos._lobbyName = _clInfos._lobbyNameDefault;
            addPopupText(gm, y + 60.f, "Create a lobby", 44.f);
            addPopupText(gm,
                y + 130.f,
                "Lobby name",
                28.f,
                raylib::Color(200, 205, 225, 255));
            const Entity field = addPopupButton(gm,
                (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f,
                y + 160.f,
                _clInfos._lobbyName,
                [](GameManager &) { activeField = Field::LobbyName; });
            const Entity create = addPopupButton(gm,
                x + CREATE_POPUP_PADDING,
                buttonsY,
                "Create",
                confirmCreate);

            gm.addComponent<TextField>(field, TextField {Field::LobbyName});
            gm.addComponent<Clock>(field);
            gm.addComponent<CreatePopupConfirm>(create);
            addPopupButton(gm,
                x + CREATE_POPUP_WIDTH - CREATE_POPUP_PADDING - BUTTON_WIDTH,
                buttonsY,
                "Back",
                closeCreatePopup);
            l->popupOpen = true;
            activeField = Field::LobbyName;
        }

        void requestJoin(GameManager &gm)
        {
            gm.view<LobbyListClient>().each([](Entity, LobbyListClient &l) {
                l.joinPort = l.selectedPort;
                l.lastRequest = {};
            });
        }

        void backToMenu(GameManager &gm)
        {
            destroyTagged<LobbyListItem>(gm);
            gm.view<LobbyListClient>().each([](Entity, LobbyListClient &l) {
                l.shown = false;
                l.wanted = false;
                l.popupOpen = false;
                l.createRequested = false;
                l.joinPort = 0;
                l.selectedPort = 0;
            });
            activeField = Field::None;
            createMenu(gm);
        }

        void switchToLobby(GameManager &gm, Entity master, uint16_t port)
        {
            destroyLobbyListPage(gm);
            gm.destroyEntity(master);
            activeField = Field::None;
            connectToServer(gm, _clInfos._ip, port);
            showConnecting(gm);
        }

        Rectangle rowRect(const std::size_t row)
        {
            return Rectangle {
                (static_cast<float>(WINDOW_WIDTH) - LOBBY_LIST_ROW_WIDTH) / 2.f,
                LOBBY_LIST_FIRST_ROW_Y +
                    static_cast<float>(row) *
                        (LOBBY_LIST_ROW_HEIGHT + LOBBY_LIST_ROW_GAP) +
                    static_cast<float>(scroll),
                LOBBY_LIST_ROW_WIDTH,
                LOBBY_LIST_ROW_HEIGHT};
        }

        bool inside(const Rectangle &rect, const Vector2 &point)
        {
            return point.x >= rect.x && point.x <= rect.x + rect.width &&
                point.y >= rect.y && point.y <= rect.y + rect.height;
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

        bool keyPressed(GameManager &gm, const Key::Value key)
        {
            bool pressed = false;

            gm.view<KeyboardManager>().each([&](Entity, KeyboardManager &k) {
                for (const auto value : k.keyPressed)
                    if (value == key)
                        pressed = true;
            });
            return pressed;
        }

        void updateScroll(GameManager &gm, const LobbyListState &list)
        {
            const int rowStep =
                static_cast<int>(LOBBY_LIST_ROW_HEIGHT + LOBBY_LIST_ROW_GAP);
            const int extra = static_cast<int>(list.lobbies.size()) -
                static_cast<int>(LOBBY_LIST_MAX_ROWS);

            if (extra <= 0) {
                scroll = 0;
                return;
            }
            gm.view<KeyboardManager>().each([](Entity, KeyboardManager &k) {
                for (const auto key : k.keyDown) {
                    if (key == Key::Value::Down)
                        scroll -= LOBBY_LIST_SCROLL_STEP;
                    if (key == Key::Value::Up)
                        scroll += LOBBY_LIST_SCROLL_STEP;
                }
            });
            gm.view<MouseManager>().each([rowStep](Entity, MouseManager &m) {
                scroll += static_cast<int>(
                    m.getWheel() * static_cast<float>(rowStep) / 2.f);
            });
            scroll = std::clamp(scroll, -extra * rowStep, 0);
        }

        void selectClickedLobby(
            GameManager &gm, LobbyListClient &l, const LobbyListState &list)
        {
            if (!leftClicked(gm))
                return;
            const Vector2 mouse = ::GetMousePosition();

            if (mouse.y < LOBBY_LIST_VISIBLE_TOP ||
                mouse.y > LOBBY_LIST_VISIBLE_BOTTOM)
                return;
            for (std::size_t i = 0; i < list.lobbies.size(); i++) {
                if (!inside(rowRect(i), mouse))
                    continue;
                l.selectedPort = l.selectedPort == list.lobbies[i].port
                    ? 0
                    : list.lobbies[i].port;
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

        template<typename Tag>
        void setDisabled(GameManager &gm, const bool disabled)
        {
            gm.view<Tag, Button>().each(
                [disabled](Entity, Tag &, Button &button) {
                    button.disabled = disabled;
                });
        }

    } // namespace

    void connectToMaster(GameManager &gm, const std::string &host, int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, host, static_cast<short>(port));
        gm.addComponent<LobbyListClient>(c);
        gm.addComponent<LobbyPortInfo>(c);
        gm.addComponent<LobbyListState>(c);
        gm.addComponent<LoginClient>(c);
        gm.addComponent<AccountResult>(c);
    }

    void openLobbyList(GameManager &gm)
    {
        destroyTagged<MenuItem>(gm);
        activeField = Field::None;
        gm.view<LobbyListClient>().each(
            [](Entity, LobbyListClient &l) { l.wanted = true; });
    }

    void createLobbyListPage(GameManager &gm)
    {
        const float buttonsY =
            static_cast<float>(WINDOW_HEIGTH) - BUTTON_HEIGHT * 2;
        const Entity title = addText(gm,
            static_cast<float>(WINDOW_WIDTH) / 2.f,
            LOBBY_LIST_TITLE_Y,
            "Lobbies",
            LOBBY_LIST_TITLE_SIZE);
        const Entity create = addButton(
            gm, BUTTON_GAP * 2, buttonsY, "Create Lobby", openCreatePopup);
        const Entity join = addButton(gm,
            (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f,
            buttonsY,
            "Join Lobby",
            requestJoin);
        const Entity leave = addButton(gm,
            static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH - BUTTON_GAP * 2,
            buttonsY,
            "Back",
            backToMenu);

        const Entity frame = gm.createEntity();

        gm.addComponent<LobbyListItem>(frame);
        gm.addComponent<LobbyListFrame>(frame);
        const Entity rows = gm.createEntity();

        gm.addComponent<LobbyListItem>(rows);
        gm.addComponent<LobbyListRowSprite>(rows);
        scroll = 0;
        gm.addComponent<LobbyListItem>(title);
        gm.addComponent<LobbyListItem>(create);
        gm.addComponent<CreateLobbyButton>(create);
        gm.addComponent<LobbyListItem>(join);
        gm.addComponent<JoinLobbyButton>(join);
        gm.addComponent<LobbyListItem>(leave);
    }

    void destroyLobbyListPage(GameManager &gm)
    {
        destroyTagged<LobbyListItem>(gm);
    }

    void systemLobbyListPage(GameManager &gm)
    {
        bool show = false;
        bool requesting = false;
        bool canJoin = false;
        bool popupOpen = false;
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
                    l.createRequested = false;
                }
                if (!l.wanted)
                    return;
                if (!l.shown) {
                    l.shown = true;
                    show = true;
                }
                if (!l.popupOpen) {
                    updateScroll(gm, list);
                    selectClickedLobby(gm, l, list);
                }
                const LobbyEntry *selected = selectedLobby(l, list);

                if (selected == nullptr)
                    l.selectedPort = 0;
                if (now - l.lastRequest >= LOBBY_LIST_RESEND_RATE) {
                    if (l.createRequested)
                        MasterProtocol::sendCreateLobby(c, l.createName);
                    else if (l.joinPort != 0)
                        MasterProtocol::sendJoinLobby(c, l.joinPort);
                    l.lastRequest = now;
                }
                requesting = l.createRequested || l.joinPort != 0;
                canJoin = selected != nullptr && !requesting &&
                    selected->players < MAX_LOBBY_PLAYERS;
                popupOpen = l.popupOpen;
            });
        if (popupOpen && keyPressed(gm, Key::Value::Enter))
            confirmCreate(gm);
        if (popupOpen && keyPressed(gm, Key::Value::Escape))
            closeCreatePopup(gm);
        if (show) {
            destroyTagged<ConnectingText>(gm);
            createLobbyListPage(gm);
        }
        setDisabled<CreateLobbyButton>(gm, requesting || popupOpen);
        setDisabled<JoinLobbyButton>(gm, !canJoin || popupOpen);
        gm.view<LobbyListItem, Button>().each(
            [&gm, popupOpen](Entity e, LobbyListItem &, Button &button) {
                if (!gm.hasComponent<CreatePopupItem>(e) &&
                    !gm.hasComponent<CreateLobbyButton>(e) &&
                    !gm.hasComponent<JoinLobbyButton>(e))
                    button.disabled = popupOpen;
            });
        gm.view<CreatePopupItem, Button>().each(
            [&gm, requesting](Entity e, CreatePopupItem &, Button &button) {
                if (!gm.hasComponent<TextField>(e))
                    button.disabled = requesting;
            });
        if (target)
            switchToLobby(gm, target->first, target->second);
    }

    void systemDrawLobbyList(GameManager &gm, Assets &assets)
    {
        const LobbyListState *state = nullptr;
        const LobbyListClient *client = nullptr;
        auto assetFont = assets.get<Font>(Asset::FONT);
        if (!assetFont)
            return;
        auto &font = assetFont.value().get();

        gm.view<LobbyListState, LobbyListClient>().each(
            [&](Entity, LobbyListState &s, LobbyListClient &l) {
                state = &s;
                client = &l;
            });
        // Only while the list page is open (the font is always loaded now).
        if (state == nullptr || client == nullptr ||
            gm.view<LobbyListItem>().getVector().empty())
            return;
        const Texture2D *rowSprite = nullptr;
        const Texture2D *frameSprite = nullptr;
        Vector2 mouse {-1.f, -1.f};

        if (!gm.view<LobbyListRowSprite>().getVector().empty()) {
            if (auto t = assets.get<Texture2D>(Asset::MENU_LOBBY_ROW))
                rowSprite = &t->get();
        }
        if (!gm.view<LobbyListFrame>().getVector().empty()) {
            if (auto t = assets.get<Texture2D>(Asset::MENU_LIST_FRAME))
                frameSprite = &t->get();
        }
        gm.view<MouseManager>().each([&mouse](Entity, MouseManager &m) {
            mouse = {m.getPos().x, m.getPos().y};
        });
        const float size = LOBBY_LIST_TEXT_SIZE;
        auto text =
            [&](const std::string &str, float tx, float ty, Color color) {
                ::DrawTextEx(font, str.c_str(), {tx, ty}, size, 1.f, color);
            };
        auto width = [&](const std::string &str) {
            return ::MeasureTextEx(font, str.c_str(), size, 1.f).x;
        };


        if (state->lobbies.empty()) {
            const std::string empty = state->received
                ? "No lobby yet, create one"
                : "Loading lobbies...";

            text(empty,
                (static_cast<float>(WINDOW_WIDTH) - width(empty)) / 2.f,
                LOBBY_LIST_FIRST_ROW_Y + 20.f,
                Color {200, 205, 225, 255});
        }
        for (std::size_t i = 0; i < state->lobbies.size(); i++) {
            const LobbyEntry &lobby = state->lobbies[i];
            const Rectangle rect = rowRect(i);
            const bool selected = lobby.port == client->selectedPort;
            const float textY = rect.y + (LOBBY_LIST_ROW_HEIGHT - size) / 2.f;
            const std::string name = lobby.name.empty()
                ? "Lobby " + std::to_string(i + 1)
                : lobby.name;
            const std::string players = std::to_string(lobby.players) + " / " +
                std::to_string(MAX_LOBBY_PLAYERS) + " players";
            const std::string status = lobby.started ? "In game" : "Waiting";

            if (rect.y + rect.height < 0.f ||
                rect.y > static_cast<float>(WINDOW_HEIGTH))
                continue;
            if (rowSprite != nullptr) {
                const bool hovered = !client->popupOpen &&
                    mouse.y >= LOBBY_LIST_VISIBLE_TOP &&
                    mouse.y <= LOBBY_LIST_VISIBLE_BOTTOM &&
                    ::CheckCollisionPointRec(mouse, rect);
                const std::size_t tile = selected ? LOBBY_ROW_TILE_SELECTED
                    : hovered                     ? LOBBY_ROW_TILE_HOVERED
                                                  : LOBBY_ROW_TILE_NORMAL;

                ::DrawTextureRec(*rowSprite,
                    {static_cast<float>(tile) * LOBBY_LIST_ROW_WIDTH,
                        0.f,
                        LOBBY_LIST_ROW_WIDTH,
                        LOBBY_LIST_ROW_HEIGHT},
                    {rect.x, rect.y},
                    WHITE);
            }
            const float nameWidth = width(name);
            const float nameSize = nameWidth > LOBBY_LIST_NAME_MAX_WIDTH
                ? size * LOBBY_LIST_NAME_MAX_WIDTH / nameWidth
                : size;

            ::DrawTextEx(font,
                name.c_str(),
                {rect.x + 30.f,
                    rect.y + (LOBBY_LIST_ROW_HEIGHT - nameSize) / 2.f},
                nameSize,
                1.f,
                WHITE);
            text(players, rect.x + LOBBY_LIST_PLAYERS_X, textY, WHITE);
            text(status,
                rect.x + LOBBY_LIST_ROW_WIDTH - 30.f - width(status),
                textY,
                lobby.started ? Color {255, 170, 60, 255}
                              : Color {90, 230, 120, 255});
        }
        if (frameSprite != nullptr)
            ::DrawTexture(*frameSprite, 0, 0, WHITE);
        if (!client->popupOpen)
            return;
        const Rectangle panel {
            (static_cast<float>(WINDOW_WIDTH) - CREATE_POPUP_WIDTH) / 2.f,
            (static_cast<float>(WINDOW_HEIGTH) - CREATE_POPUP_HEIGHT) / 2.f,
            CREATE_POPUP_WIDTH,
            CREATE_POPUP_HEIGHT};

        ::DrawRectangle(
            0, 0, WINDOW_WIDTH, WINDOW_HEIGTH, Color {0, 0, 0, 160});
        ::DrawRectangleRec(panel, Color {30, 36, 70, 255});
        ::DrawRectangleLinesEx(panel, 4.f, Color {200, 200, 220, 255});
    }

} // namespace RType::Client
