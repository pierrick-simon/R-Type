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
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        const raylib::Color COLOR_INFO(200, 205, 225, 255);
        const raylib::Color COLOR_TITLE(255, 95, 95, 255);

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

        void addGameOverButton(GameManager &gm, float x,
            const std::string &label,
            std::function<void(GameManager &)> onClick, GameOverChoice choice)
        {
            const Entity e = addButton(
                gm, x, GAME_OVER_BUTTONS_Y, label, std::move(onClick));

            gm.addComponent<GameOverItem>(e);
            gm.addComponent<GameOverButton>(e, GameOverButton {choice});
        }

        bool hasVoted(const LobbyState &state, const uint16_t myId)
        {
            return std::ranges::any_of(
                state.players, [myId](const LobbyPlayerInfo &p) {
                    return p.id == myId && p.replay;
                });
        }

        /** @brief Sends the choice until the server takes it into account. */
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
                    button.disabled =
                        tag.choice == GameOverChoice::Restart ? voted : !isHost;
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
        const float x = center - BUTTON_WIDTH - BUTTON_GAP / 2.f;
        const Entity background = gm.createEntity();

        gm.addComponent<GameOverItem>(background);
        gm.addComponent<Sprite2D>(background, 1.f);
        gm.addComponent<Texture2D>(
            background, gm.getAssetsMap().at(Asset::MENU_BACKGROUND));
        gm.addComponent<Position2D>(background, 0.f, 0.f);
        gm.addComponent<GameOverItem>(addText(gm,
            center,
            GAME_OVER_TITLE_Y,
            "GAME OVER",
            TITLE_SIZE,
            {},
            COLOR_TITLE));
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
            x + BUTTON_WIDTH + BUTTON_GAP,
            "Back to Lobby",
            requestReturn,
            GameOverChoice::Return);
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
                    hide = lobby.gameOverShown;
                    lobby.gameOverShown = false;
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

} // namespace RType::Client
