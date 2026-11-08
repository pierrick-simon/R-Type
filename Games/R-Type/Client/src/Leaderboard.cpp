/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Leaderboard
*/

#include "Leaderboard.hpp"

#include <string>
#include <vector>

#include "Assets.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "Lobby.hpp"
#include "Login.hpp"
#include "Menu.hpp"
#include "Position2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"

namespace RType::Client {

    namespace {

        float rowX()
        {
            return (static_cast<float>(WINDOW_WIDTH) - LOBBY_ROW_WIDTH) / 2.f;
        }

        float rowTop(const std::size_t row)
        {
            if (row == LEADERBOARD_YOU_ROW)
                return LEADERBOARD_YOU_ROW_Y;
            return LEADERBOARD_FIRST_ROW_Y +
                static_cast<float>(row) * (LOBBY_ROW_HEIGHT + LOBBY_ROW_GAP);
        }

        std::string line(const LeaderboardEntry &entry)
        {
            return std::to_string(entry.rank) + " - " + entry.name + " - " +
                std::to_string(entry.score);
        }

        Entity addPageText(GameManager &gm, float y, const std::string &str,
            float size, TextAnchor anchor = {0.5f, 0.5f})
        {
            const Entity e = addText(gm,
                static_cast<float>(WINDOW_WIDTH) / 2.f,
                y,
                str,
                size,
                anchor);

            gm.addComponent<LeaderboardItem>(e);
            return e;
        }

        /** @brief Row background and its text, both hidden until filled. */
        void addRow(GameManager &gm, const std::size_t row)
        {
            const Entity back = gm.createEntity();
            SpriteSheet2D sheet(
                Size2D {1, 2}, LOBBY_ROW_TILE_OTHER, 0, 1.f, false);

            sheet.color.a = 0;
            sheet.assetId = Asset::MENU_PLAYER_BACK;
            gm.addComponent<LeaderboardItem>(back);
            gm.addComponent<LeaderboardRowBack>(back, LeaderboardRowBack {row});
            gm.addComponent<SpriteSheet2D>(back, sheet);
            gm.addComponent<Position2D>(back, rowX(), rowTop(row));
            const Entity text = addPageText(gm,
                rowTop(row) + LOBBY_ROW_HEIGHT / 2.f,
                row == 0 ? "Loading..." : "",
                LEADERBOARD_TEXT_SIZE);

            gm.addComponent<LeaderboardRowText>(
                text, LeaderboardRowText {row});
        }

        void showRow(GameManager &gm, const std::size_t row,
            const std::string &str, const bool visible, const bool me)
        {
            gm.view<LeaderboardRowBack, SpriteSheet2D>().each(
                [&](Entity, LeaderboardRowBack &back, SpriteSheet2D &sheet) {
                    if (back.row != row)
                        return;
                    sheet.color.a = visible ? 255 : 0;
                    sheet.currentTile =
                        me ? LOBBY_ROW_TILE_ME : LOBBY_ROW_TILE_OTHER;
                });
            gm.view<LeaderboardRowText, Text2D>().each(
                [&](Entity, LeaderboardRowText &text, Text2D &t) {
                    if (text.row == row)
                        t.str = str;
                });
        }

        void fillRows(GameManager &gm, const LeaderboardState &state)
        {
            const std::string me = _account.loggedIn ? _account.username : "";

            for (std::size_t row = 0; row < LEADERBOARD_SIZE; row++) {
                if (row < state.top.size())
                    showRow(gm,
                        row,
                        line(state.top[row]),
                        true,
                        state.top[row].name == me);
                else
                    showRow(gm,
                        row,
                        row == 0 && state.top.empty() ? "No score yet" : "",
                        false,
                        false);
            }
            if (state.you)
                showRow(gm, LEADERBOARD_YOU_ROW, line(*state.you), true, true);
            else
                showRow(gm,
                    LEADERBOARD_YOU_ROW,
                    "Log in to see your score",
                    true,
                    false);
        }

        void createLeaderboardPage(GameManager &gm)
        {
            addPageText(
                gm, LEADERBOARD_TITLE_Y, "Leaderboard", LOBBY_TITLE_SIZE);
            for (std::size_t row = 0; row < LEADERBOARD_SIZE; row++)
                addRow(gm, row);
            addPageText(gm, LEADERBOARD_YOU_LABEL_Y, "Your Score", 30.f);
            addRow(gm, LEADERBOARD_YOU_ROW);
            const Entity back = addButton(gm,
                static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH -
                    BUTTON_GAP * 2,
                static_cast<float>(WINDOW_HEIGTH) - BUTTON_HEIGHT * 2 -
                    BUTTON_GAP,
                "Back",
                closeLeaderboard);

            gm.addComponent<LeaderboardItem>(back);
        }

    } // namespace

    void openLeaderboard(GameManager &gm)
    {
        std::vector<Entity> items;

        gm.view<MenuItem>().each(
            [&items](Entity e, MenuItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
        activeField = Field::None;
        gm.view<LeaderboardClient, LeaderboardState>().each(
            [](Entity, LeaderboardClient &client, LeaderboardState &state) {
                client = {true, false, {}};
                state = {};
            });
        createLeaderboardPage(gm);
    }

    void closeLeaderboard(GameManager &gm)
    {
        std::vector<Entity> items;

        gm.view<LeaderboardItem>().each(
            [&items](Entity e, LeaderboardItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
        gm.view<LeaderboardClient>().each(
            [](Entity, LeaderboardClient &client) { client.shown = false; });
        createMenu(gm);
    }

    void systemLeaderboard(GameManager &gm)
    {
        gm.view<ClientManager, LeaderboardClient, LeaderboardState>().each(
            [&gm](Entity,
                ClientManager &c,
                LeaderboardClient &client,
                LeaderboardState &state) {
                if (!client.shown || client.answered)
                    return;
                if (state.received) {
                    client.answered = true;
                    fillRows(gm, state);
                    return;
                }
                const auto now = std::chrono::steady_clock::now();

                if (c.isConnected() &&
                    now - client.lastRequest >= LEADERBOARD_RESEND_RATE) {
                    AccountProtocol::sendLeaderboardRequest(c);
                    client.lastRequest = now;
                }
            });
    }

} // namespace RType::Client
