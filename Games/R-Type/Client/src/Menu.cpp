/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#include <vector>

#include "ClientManager.hpp"
#include "Menu.hpp"
#include "Text2D.hpp"
#include "Assets.hpp"
#include "Clock.hpp"

namespace RType::Client {

    void showConnecting(GameManager &gm)
    {
        const Entity e = gm.createEntity();
        Text2D text;

        text.fontSize = 40;
        text.str = "Connexion";
        gm.addComponent<ConnectingText>(e);
        gm.addComponent<Clock>(e);
        gm.addComponent<Text2D>(e, text);
        gm.addComponent<Font>(e, gm.getAssetsMap().at(Asset::FONT));
        gm.addComponent<Position2D>(e, 1920.f / 2.f - 230.f, 1080.f / 2.f - 30.f);

    }

    void systemConnecting(GameManager &gm)
    {
        bool connected = false;

        gm.view<ClientManager>().each([&](Entity, ClientManager &c) {
            connected = !c.getContext().map.empty();
        });
        gm.view<ConnectingText, Text2D, Clock>().each([](Entity, ConnectingText &c, Text2D &text, Clock &clock) {
            c.elapsed += clock.getElapsed();
            const auto dots = (c.elapsed.count() / 400) % 4;
            text.str = "Connexion" + std::string(static_cast<std::size_t>(dots), '.');
        });        
        if (!connected)
            return;
        gm.view<ConnectingText>().each(
            [&gm](Entity e, ConnectingText &) { gm.destroyEntity(e); });
    }

    void systemChangingUsername(GameManager &gm)
    {
        if (!changingUsername)
            return;
        for (int key = ::GetCharPressed(); key > 0; key = ::GetCharPressed()) {
            if (key >= 32 && key <= 125
                && _clInfos._username.size() < USERNAME_MAX_LENGTH)
                _clInfos._username += static_cast<char>(key);
        }
        if (::IsKeyPressed(KEY_BACKSPACE) && !_clInfos._username.empty())
            _clInfos._username.pop_back();
        if (::IsKeyPressed(KEY_ENTER) || ::IsKeyPressed(KEY_ESCAPE)) {
            changingUsername = false;
            if (_clInfos._username.empty())
                _clInfos._username = "Player";
        }
        gm.view<Username, Button, Clock>().each(
            [](Entity, Username &u, Button &button, Clock clock) {
                u.elapsed += clock.getElapsed();
                const bool blink = changingUsername
                    && (u.elapsed.count() / 400) % 2 == 0;
                button.label = _clInfos._username + (blink ? "_" : "");
            });
    }

    void systemServerLost(GameManager &gm)
    {
        std::vector<Entity> toDestroy;
        bool lost = false;

        gm.view<ClientManager>().each([&](Entity e, ClientManager &c) {
            if (!c.isTimedOut())
                return;
            lost = true;
            toDestroy.push_back(e);
            for (auto &[id, entity] : c.getContext().map)
                if (gm.hasEntity(entity))
                    toDestroy.push_back(entity);
        });
        if (!lost)
            return;
        gm.view<ConnectingText>().each(
            [&](Entity e, ConnectingText &) { toDestroy.push_back(e); });
        for (const Entity e : toDestroy)
            gm.destroyEntity(e);
        createMenu(gm);
    }

    Entity addButton(GameManager &gm, float x, float y, const std::string &label,
        std::function<void(GameManager &)> onClick)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<MenuItem>(e);
        gm.addComponent<Button>(e,
            Button {{x, y, BUTTON_WIDTH, BUTTON_HEIGHT},
                label,
                std::move(onClick),
                false});
        return e;
    }

    void startGame(GameManager &gm)
    {
        if (!gm.view<ClientManager>().getVector().empty())
            return;
        std::vector<Entity> items;

        gm.view<MenuItem>().each(
            [&items](Entity e, MenuItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
        connectToServer(gm, LOGIN_SERVER_PORT);
        showConnecting(gm);
    }

    void createMenu(GameManager &gm)
    {
        const float firstY = 480.f;
        const float firstX = (1920.f - BUTTON_WIDTH) / 2.f;

        addButton(gm, firstX, firstY, "Jouer", startGame);
        const Entity usernameButton = addButton(gm, 20 , 1080 - (BUTTON_HEIGHT * 2), _clInfos._username,
            [](GameManager &) { changingUsername = true; });
        gm.addComponent<Username>(usernameButton);
        gm.addComponent<Clock>(usernameButton);
        addButton(gm, firstX, firstY + BUTTON_HEIGHT + BUTTON_GAP, "Quitter", [](GameManager &) { quitRequested = true; });
    }

    void connectToServer(GameManager &gm, const int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, "127.0.0.1", port);
    }

    void systemButtons(GameManager &gm)
    {
        const Vector2 mouse = ::GetMousePosition();
        const bool pressed = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        std::vector<std::function<void(GameManager &)>> clicked;

        if (changingUsername && pressed) {
            bool onUsernameButton = false;

            gm.view<Username, Button>().each(
                [&](Entity, Username &, Button &button) {
                    onUsernameButton =
                        ::CheckCollisionPointRec(mouse, button.rect);
                });
            if (!onUsernameButton) {
                changingUsername = false;
                if (_clInfos._username.empty())
                    _clInfos._username = "Player";
                return;
            }
        }
        gm.view<Button>().each([&](Entity, Button &button) {
            button.hovered = ::CheckCollisionPointRec(mouse, button.rect);
            if (button.hovered && pressed)
                clicked.push_back(button.onClick);
        });
        for (auto &callback : clicked)
            callback(gm);
    }

    void systemDrawMenu(GameManager &gm)
    {
        const auto buttons = gm.view<Button>().getVector();

        if (buttons.empty())
            return;
        const char *title = "R-TYPE";
        const int titleWidth = ::MeasureText(title, TITLE_SIZE);

        DrawText(title, (1920 - titleWidth) / 2, 200, TITLE_SIZE, WHITE);
        gm.view<Button>().each([](Entity, Button &button) {
            const Color fill = button.hovered ? Color {70, 120, 220, 255}
                                              : Color {40, 50, 90, 255};
            const int textWidth =
                ::MeasureText(button.label.c_str(), LABEL_SIZE);

            ::DrawRectangleRec(button.rect, fill);
            ::DrawRectangleLinesEx(button.rect, 3.f, WHITE);
            ::DrawText(button.label.c_str(),
                static_cast<int>(button.rect.x + (button.rect.width - textWidth) / 2),
                static_cast<int>(button.rect.y + (button.rect.height - LABEL_SIZE) / 2),
                LABEL_SIZE,
                WHITE);
        });
    }

} // namespace RType::Client
