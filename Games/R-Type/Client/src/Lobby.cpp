/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lobby
*/

#include "Lobby.hpp"

#include <string>
#include <vector>

#include "Assets.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "LobbyProtocol.hpp"
#include "Menu.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        const raylib::Color COLOR_INFO(200, 205, 225, 255);
        const raylib::Color COLOR_READY(90, 230, 120, 255);
        const raylib::Color COLOR_NOT_READY(255, 95, 95, 255);
        const raylib::Color COLOR_HOST(255, 215, 0, 255);

        template<typename Component>
        void collect(GameManager &gm, std::vector<Entity> &out)
        {
            gm.view<Component>().each(
                [&out](Entity e, Component &) { out.push_back(e); });
        }

        float rowX()
        {
            return (static_cast<float>(WINDOW_WIDTH) - LOBBY_ROW_WIDTH) / 2.f;
        }

        float rowCenterY(const std::size_t row)
        {
            return LOBBY_FIRST_ROW_Y
                + static_cast<float>(row) * (LOBBY_ROW_HEIGHT + LOBBY_ROW_GAP)
                + LOBBY_ROW_HEIGHT / 2.f;
        }

        void addLobbyText(GameManager &gm, float x, float y, float size,
            TextAnchor anchor, LobbyText id,
            raylib::Color color = raylib::Color::White())
        {
            const Entity e = addText(gm, x, y, "", size, anchor, color);

            gm.addComponent<LobbyItem>(e);
            gm.addComponent<LobbyText>(e, id);
        }

        void addLobbyButton(GameManager &gm, float x, const std::string &label,
            std::function<void(GameManager &)> onClick, LobbyButtonId id)
        {
            const Entity e =
                addButton(gm, x, LOBBY_BUTTONS_Y, label, std::move(onClick));

            gm.addComponent<LobbyItem>(e);
            gm.addComponent<LobbyButton>(e, LobbyButton {id});
        }

        void addRow(GameManager &gm, const std::size_t row)
        {
            const Entity back = gm.createEntity();
            const float y = rowCenterY(row);
            SpriteSheet2D sheet(Size2D {1, 2}, LOBBY_ROW_TILE_OTHER, 0, 1.f, false);

            sheet.color.a = 0;
            gm.addComponent<LobbyItem>(back);
            gm.addComponent<LobbyRow>(back, LobbyRow {row});
            gm.addComponent<SpriteSheet2D>(back, sheet);
            gm.addComponent<Texture2D>(
                back, gm.getAssetsMap().at(Asset::MENU_PLAYER_BACK));
            gm.addComponent<Position2D>(back, rowX(), y - LOBBY_ROW_HEIGHT / 2.f);
            addLobbyText(gm, rowX() + LOBBY_ROW_PADDING, y, LOBBY_NAME_SIZE,
                TextAnchor {0.f, 0.5f}, LobbyText {LobbyTextId::RowName, row});
            addLobbyText(gm, rowX() + LOBBY_ROW_PADDING, y, LOBBY_HOST_SIZE,
                TextAnchor {0.f, 0.5f}, LobbyText {LobbyTextId::RowHost, row},
                COLOR_HOST);
            addLobbyText(gm, rowX() + LOBBY_ROW_WIDTH - LOBBY_ROW_PADDING, y,
                LOBBY_NAME_SIZE, TextAnchor {1.f, 0.5f},
                LobbyText {LobbyTextId::RowStatus, row});
        }

        void toggleReady(GameManager &gm)
        {
            gm.view<ClientManager, LobbyClient>().each(
                [](Entity, ClientManager &c, LobbyClient &l) {
                    l.ready = !l.ready;
                    LobbyProtocol::sendInfo(c, _clInfos._username, l.ready);
                    l.lastInfo = std::chrono::steady_clock::now();
                });
        }

        void requestStart(GameManager &gm)
        {
            gm.view<LobbyClient>().each([](Entity, LobbyClient &l) {
                l.startRequested = true;
                l.lastStart = {};
            });
        }

        int countReady(const LobbyState &state)
        {
            int n = 0;

            for (const auto &p : state.players)
                n += p.ready ? 1 : 0;
            return n;
        }

        std::string statusLine(
            const LobbyState &state, const bool isHost, raylib::Color &color)
        {
            const int total = static_cast<int>(state.players.size());

            color = state.allReady() ? COLOR_READY : COLOR_INFO;
            if (state.allReady())
                return isHost ? "Everyone is ready, start the game!"
                              : "Everyone is ready, waiting for the host...";
            return "Players ready  " + std::to_string(countReady(state)) + " / "
                + std::to_string(total);
        }

        void updateButtons(GameManager &gm, const LobbyState &state,
            const LobbyClient &lobby, const bool isHost)
        {
            gm.view<LobbyButton, Button>().each(
                [&](Entity, LobbyButton &tag, Button &button) {
                    if (tag.id == LobbyButtonId::Ready)
                        button.label = lobby.ready ? "Not Ready" : "Ready";
                    if (tag.id == LobbyButtonId::Start)
                        button.disabled = !isHost || !state.allReady();
                });
        }

        void updateRows(
            GameManager &gm, const LobbyState &state, const uint16_t myId)
        {
            gm.view<LobbyRow, SpriteSheet2D>().each(
                [&](Entity, LobbyRow &row, SpriteSheet2D &sheet) {
                    const bool used = row.row < state.players.size();

                    sheet.color.a = used ? 255 : 0;
                    sheet.currentTile = used && state.players[row.row].id == myId
                        ? LOBBY_ROW_TILE_ME
                        : LOBBY_ROW_TILE_OTHER;
                });
        }

        void updateTexts(
            GameManager &gm, const LobbyState &state, const bool isHost)
        {
            // Room left for the name between the row start and the "HOST"
            // tag + "NOT READY" status: longer names get a smaller font.
            constexpr float NAME_MAX_WIDTH = 360.f;
            const int total = static_cast<int>(state.players.size());
            std::vector<float> nameWidth(LOBBY_MAX_ROWS, 0.f);

            gm.view<LobbyText, Text2D, Font>().each(
                [&](Entity, LobbyText &id, Text2D &text, Font &font) {
                    const bool used = id.row < state.players.size();
                    const LobbyPlayerInfo *p =
                        used ? &state.players[id.row] : nullptr;

                    switch (id.id) {
                        case LobbyTextId::Subtitle:
                            text.str = "Server " + _clInfos._ip + "   " + std::to_string(total) + (total > 1 ? " players" : " player");
                            break;
                        case LobbyTextId::Status:
                            text.str = statusLine(state, isHost, text.color);
                            break;
                        case LobbyTextId::RowName: {
                            float width = 0.f;

                            text.str = p ? p->name : "";
                            text.fontSize = LOBBY_NAME_SIZE;
                            width = font.MeasureText(
                                text.str, text.fontSize, text.spacing).x;
                            if (width > NAME_MAX_WIDTH) {
                                text.fontSize *= NAME_MAX_WIDTH / width;
                                width = NAME_MAX_WIDTH;
                            }
                            if (id.row < nameWidth.size())
                                nameWidth[id.row] = width;
                            break;
                        }
                        case LobbyTextId::RowHost:
                            text.str = p && p->id == state.hostId ? "HOST" : "";
                            break;
                        case LobbyTextId::RowStatus:
                            text.str = p ? (p->ready ? "READY" : "NOT READY") : "";
                            text.color = p && p->ready ? COLOR_READY
                                                       : COLOR_NOT_READY;
                            break;
                    }
                });
            gm.view<LobbyText, Position2D>().each(
                [&](Entity, LobbyText &id, Position2D &pos) {
                    if (id.id == LobbyTextId::RowHost && id.row < nameWidth.size())
                        pos.x = rowX() + LOBBY_ROW_PADDING + nameWidth[id.row] + 20.f;
                });
        }

    } // namespace

    void createLobby(GameManager &gm)
    {
        const float center = static_cast<float>(WINDOW_WIDTH) / 2.f;
        const float x = center - (3.f * BUTTON_WIDTH + 2.f * BUTTON_GAP) / 2.f;
        const Entity title =
            addText(gm, center, LOBBY_TITLE_Y, "LOBBY", LOBBY_TITLE_SIZE);

        activeField = Field::None;
        gm.addComponent<LobbyItem>(title);
        addLobbyText(gm, center, LOBBY_SUBTITLE_Y, LOBBY_INFO_SIZE, TextAnchor {},
            LobbyText {LobbyTextId::Subtitle}, COLOR_INFO);
        for (std::size_t row = 0; row < LOBBY_MAX_ROWS; row++)
            addRow(gm, row);
        addLobbyText(gm, center, LOBBY_STATUS_Y, LOBBY_INFO_SIZE,
            TextAnchor {}, LobbyText {LobbyTextId::Status}, COLOR_INFO);
        addLobbyButton(gm, x, "Ready", toggleReady, LobbyButtonId::Ready);
        addLobbyButton(gm, x + BUTTON_WIDTH + BUTTON_GAP, "Start Game",
            requestStart, LobbyButtonId::Start);
        addLobbyButton(gm, x + 2.f * (BUTTON_WIDTH + BUTTON_GAP), "Leave Game",
            returnToMenu, LobbyButtonId::Leave);
    }

    void destroyLobby(GameManager &gm)
    {
        std::vector<Entity> items;

        collect<LobbyItem>(gm, items);
        for (const Entity e : items)
            gm.destroyEntity(e);
    }

    void returnToMenu(GameManager &gm)
    {
        std::vector<Entity> toDestroy;

        gm.view<ClientManager>().each([&](Entity e, ClientManager &c) {
            toDestroy.push_back(e);
            for (auto &[id, entity] : c.getContext().map)
                if (gm.hasEntity(entity))
                    toDestroy.push_back(entity);
        });
        collect<LobbyItem>(gm, toDestroy);
        collect<ConnectingText>(gm, toDestroy);
        for (const Entity e : toDestroy)
            gm.destroyEntity(e);
        activeField = Field::None;
        createMenu(gm);
    }

    void systemLobby(GameManager &gm)
    {
        bool show = false;
        bool hide = false;

        gm.view<ClientManager, LobbyState, LobbyClient>().each(
            [&](Entity, ClientManager &c, LobbyState &state, LobbyClient &lobby) {
                const auto now = std::chrono::steady_clock::now();

                if (c.isConnected() && now - lobby.lastInfo >= LOBBY_RESEND_RATE) {
                    LobbyProtocol::sendInfo(c, _clInfos._username, lobby.ready);
                    lobby.lastInfo = now;
                }
                if (!state.received || lobby.inGame)
                    return;
                if (state.started) {
                    lobby.inGame = true;
                    hide = lobby.shown;
                    lobby.shown = false;
                    return;
                }
                if (!lobby.shown) {
                    lobby.shown = true;
                    show = true;
                }
                const bool isHost = state.hostId == c.getMyId();

                if (!isHost || !state.allReady())
                    lobby.startRequested = false;
                if (lobby.startRequested
                    && now - lobby.lastStart >= LOBBY_RESEND_RATE) {
                    LobbyProtocol::sendStart(c);
                    lobby.lastStart = now;
                }
                updateButtons(gm, state, lobby, isHost);
                updateRows(gm, state, c.getMyId());
                updateTexts(gm, state, isHost);
            });
        if (show)
            createLobby(gm);
        if (hide)
            destroyLobby(gm);
    }

} // namespace RType::Client
