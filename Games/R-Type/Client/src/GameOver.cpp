/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameOver
*/

#include "GameOver.hpp"

#include <chrono>
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

        const raylib::Color COLOR_WAITING(200, 205, 225, 255);
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
            std::function<void(GameManager &)> onClick)
        {
            const Entity e = addButton(
                gm, x, GAME_OVER_BUTTONS_Y, label, std::move(onClick));

            gm.addComponent<GameOverItem>(e);
            gm.addComponent<GameOverButton>(e);
        }

        /** @brief Sends the host's choice until the server acts on it. */
        void sendChoice(ClientManager &c, LobbyClient &lobby)
        {
            const auto now = std::chrono::steady_clock::now();

            if (lobby.choice == GameOverChoice::None ||
                now - lobby.lastChoice < LOBBY_RESEND_RATE)
                return;
            lobby.lastChoice = now;
            if (lobby.choice == GameOverChoice::Restart)
                LobbyProtocol::sendRestart(c);
            else
                LobbyProtocol::sendReturn(c);
        }

        void updateGameOver(GameManager &gm, const bool isHost)
        {
            gm.view<GameOverButton, Button>().each(
                [isHost](Entity, GameOverButton &, Button &button) {
                    button.disabled = !isHost;
                });
            gm.view<GameOverStatus, Text2D>().each(
                [isHost](Entity, GameOverStatus &, Text2D &text) {
                    text.str = isHost ? "" : "Waiting for the host...";
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
            COLOR_WAITING);

        gm.addComponent<GameOverItem>(status);
        gm.addComponent<GameOverStatus>(status);
        addGameOverButton(gm, x, "Play Again", requestRestart);
        addGameOverButton(
            gm, x + BUTTON_WIDTH + BUTTON_GAP, "Back to Lobby", requestReturn);
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
                isHost = state.hostId == c.getMyId();
                show = !lobby.gameOverShown;
                lobby.gameOverShown = true;
                if (isHost)
                    sendChoice(c, lobby);
            });
        if (show)
            createGameOver(gm);
        if (hide)
            destroyGameOver(gm);
        if (!hide)
            updateGameOver(gm, isHost);
    }

} // namespace RType::Client
