/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameOver
*/

#include "GameOver.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "Assets.hpp"
#include "Client.hpp"
#include "ClientManager.hpp"
#include "Lobby.hpp"
#include "LobbyProtocol.hpp"
#include "Menu.hpp"
#include "Sprite2D.hpp"
#include "Text2D.hpp"

namespace RType::Client {

    namespace {

        const raylib::Color COLOR_INFO(200, 205, 225, 255);
        const raylib::Color COLOR_TITLE(255, 95, 95, 255);
        const raylib::Color COLOR_VICTORY(90, 230, 120, 255);

        void choose(GameManager &gm, const GameOverChoice choice)
        {
            gm.view<LobbyClient>().each([choice](Entity, LobbyClient &l) {
                l.choice = choice;
                l.lastChoice = {};
            });
        }

        void requestRestart(GameManager &gm)
        {
            choose(gm, GameOverChoice::Restart);
        }

        void requestReturn(GameManager &gm)
        {
            choose(gm, GameOverChoice::Return);
        }

        void leaveToMenu(GameManager &gm)
        {
            destroyGameOver(gm);
            returnToMenu(gm);
        }

        void addGameOverButton(GameManager &gm, float x,
            const std::string &label,
            std::function<void(GameManager &)> onClick, GameOverChoice choice)
        {
            const Entity e = addButton(
                gm, x, GAME_OVER_BUTTONS_Y, label, std::move(onClick));

            gm.addComponent<GameOverItem>(e);
            gm.addComponent<GameOverButton>(e, GameOverButton {choice});
        }

        float secondsSince(std::chrono::steady_clock::time_point start)
        {
            return std::chrono::duration<float>(
                std::chrono::steady_clock::now() - start)
                .count();
        }

        float fadeAlpha(const LobbyClient &lobby)
        {
            if (!lobby.fadeStart)
                return 0.f;
            const float t = secondsSince(*lobby.fadeStart);

            if (t < VICTORY_FADE_SECONDS)
                return t / VICTORY_FADE_SECONDS;
            if (t < VICTORY_FADE_SECONDS + VICTORY_HOLD_SECONDS)
                return 1.f;
            return std::max(0.f,
                1.f -
                    (t - VICTORY_FADE_SECONDS - VICTORY_HOLD_SECONDS) /
                        VICTORY_FADE_IN_SECONDS);
        }

        bool hasVoted(const LobbyState &state, const uint16_t myId)
        {
            return std::ranges::any_of(
                state.players, [myId](const LobbyPlayerInfo &p) {
                    return p.id == myId && p.replay;
                });
        }

        void sendChoice(ClientManager &c, LobbyClient &lobby, bool voted)
        {
            const auto now = std::chrono::steady_clock::now();

            if (lobby.choice == GameOverChoice::None ||
                (lobby.choice == GameOverChoice::Restart && voted) ||
                now - lobby.lastChoice < LOBBY_RESEND_RATE)
                return;
            lobby.lastChoice = now;
            if (lobby.choice == GameOverChoice::Restart)
                LobbyProtocol::sendRestart(c);
            else
                LobbyProtocol::sendReturn(c);
        }

        void updateGameOver(GameManager &gm, const LobbyState &state,
            const bool isHost, const bool voted)
        {
            const std::string votes = std::to_string(state.replayVotes()) +
                " / " + std::to_string(state.players.size());

            gm.view<GameOverButton, Button>().each(
                [&](Entity, GameOverButton &tag, Button &button) {
                    if (tag.choice == GameOverChoice::Restart)
                        button.disabled = voted;
                    if (tag.choice == GameOverChoice::Return)
                        button.disabled = !isHost;
                });
            gm.view<GameOverTitle, Text2D>().each(
                [&](Entity, GameOverTitle &, Text2D &text) {
                    text.str = state.victory ? "VICTORY" : "GAME OVER";
                    text.color = state.victory ? COLOR_VICTORY : COLOR_TITLE;
                });
            gm.view<GameOverStatus, Text2D>().each(
                [&](Entity, GameOverStatus &, Text2D &text) {
                    text.str = "Play again  " + votes;
                });
        }

    } // namespace

    void createGameOver(GameManager &gm)
    {
        const float center = static_cast<float>(WINDOW_WIDTH) / 2.f;
        const float x = center - (3.f * BUTTON_WIDTH + 2.f * BUTTON_GAP) / 2.f;
        const float step = BUTTON_WIDTH + BUTTON_GAP;
        const Entity background = gm.createEntity();

        gm.addComponent<GameOverItem>(background);
        gm.addComponent<Sprite2D>(background, Asset::MENU_BACK);
        gm.addComponent<Position2D>(background, 0.f, 0.f);
        const Entity title = addText(
            gm, center, GAME_OVER_TITLE_Y, "", TITLE_SIZE, {}, COLOR_TITLE);

        gm.addComponent<GameOverItem>(title);
        gm.addComponent<GameOverTitle>(title);
        const Entity status = addText(gm,
            center,
            GAME_OVER_STATUS_Y,
            "",
            GAME_OVER_STATUS_SIZE,
            {},
            COLOR_INFO);

        gm.addComponent<GameOverItem>(status);
        gm.addComponent<GameOverStatus>(status);
        addGameOverButton(
            gm, x, "Play Again", requestRestart, GameOverChoice::Restart);
        addGameOverButton(gm,
            x + step,
            "Back to Lobby",
            requestReturn,
            GameOverChoice::Return);
        addGameOverButton(gm,
            x + 2.f * step,
            "Main Menu",
            leaveToMenu,
            GameOverChoice::Leave);
    }

    void destroyGameOver(GameManager &gm)
    {
        std::vector<Entity> items;

        gm.view<GameOverItem>().each(
            [&items](Entity e, GameOverItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
    }

    void systemGameOver(GameManager &gm)
    {
        bool show = false;
        bool hide = false;
        bool isHost = false;
        bool voted = false;
        LobbyState current;

        gm.view<ClientManager, LobbyState, LobbyClient>().each(
            [&](Entity,
                ClientManager &c,
                LobbyState &state,
                LobbyClient &lobby) {
                if (!lobby.inGame || !state.gameOver) {
                    lobby.choice = GameOverChoice::None;
                    lobby.fadeStart.reset();
                    hide = lobby.gameOverShown;
                    lobby.gameOverShown = false;
                    return;
                }
                if (state.victory && !lobby.gameOverShown) {
                    if (!lobby.fadeStart)
                        lobby.fadeStart = std::chrono::steady_clock::now();
                    if (secondsSince(*lobby.fadeStart) <
                        VICTORY_FADE_SECONDS + VICTORY_HOLD_SECONDS)
                        return;
                }
                current = state;
                isHost = state.hostId == c.getMyId();
                voted = hasVoted(state, c.getMyId());
                show = !lobby.gameOverShown;
                lobby.gameOverShown = true;
                sendChoice(c, lobby, voted);
            });
        if (show)
            createGameOver(gm);
        if (hide)
            destroyGameOver(gm);
        if (!hide)
            updateGameOver(gm, current, isHost, voted);
    }

    void systemDrawFade(LobbyClient &lobby)
    {
        const float alpha = fadeAlpha(lobby);

        if (alpha <= 0.f)
            return;
        ::DrawRectangle(0,
            0,
            WINDOW_WIDTH,
            WINDOW_HEIGTH,
            {0, 0, 0, static_cast<unsigned char>(alpha * 255.f)});
    }

} // namespace RType::Client
