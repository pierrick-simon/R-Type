/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MenuSystems
*/

#include <vector>

#include "ClientManager.hpp"
#include "Lobby.hpp"
#include "LobbyProtocol.hpp"
#include "Menu.hpp"
#include "Text2D.hpp"
#include "Assets.hpp"
#include "Clock.hpp"
#include "MouseManager.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        std::string &fieldValue(const Field field)
        {
            return field == Field::Ip ? _clInfos._ip : _clInfos._username;
        }

        const char *fieldFallback(const Field field)
        {
            return field == Field::Ip ? "127.0.0.1" : "Player";
        }

        std::size_t fieldMaxLength(const Field field)
        {
            return field == Field::Ip ? IP_MAX_LENGTH : USERNAME_MAX_LENGTH;
        }

        bool fieldAccepts(const Field field, const int key)
        {
            if (field == Field::Ip)
                return (key >= '0' && key <= '9') || key == '.';
            return key >= 32 && key <= 125;
        }

        bool contains(const Rectangle &rect, const Vector2 &point)
        {
            return point.x >= rect.x && point.x <= rect.x + rect.width
                && point.y >= rect.y && point.y <= rect.y + rect.height;
        }

        void stopEditing()
        {
            if (activeField == Field::None)
                return;
            if (fieldValue(activeField).empty())
                fieldValue(activeField) = fieldFallback(activeField);
            activeField = Field::None;
        }


    } // namespace


    void systemConnecting(GameManager &gm)
    {
        bool connected = false;

        gm.view<ClientManager, LobbyState>().each(
            [&](Entity, ClientManager &, LobbyState &state) {
                connected = state.received;
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

    void systemTextFields(GameManager &gm)
    {
        if (activeField != Field::None) {
            std::string &value = fieldValue(activeField);

            if (activeField == Field::Ip)
                ipInvalid = false;
            for (int key = ::GetCharPressed(); key > 0;
                key = ::GetCharPressed()) {
                if (fieldAccepts(activeField, key)
                    && value.size() < fieldMaxLength(activeField))
                    value += static_cast<char>(key);
            }
            if (::IsKeyPressed(KEY_BACKSPACE) && !value.empty())
                value.pop_back();
            if (::IsKeyPressed(KEY_ENTER) || ::IsKeyPressed(KEY_ESCAPE))
                stopEditing();
        }
        gm.view<TextField, Button, Clock>().each(
            [](Entity, TextField &f, Button &button, Clock &clock) {
                f.elapsed += clock.getElapsed();
                const bool blink = activeField == f.id
                    && (f.elapsed.count() / 400) % 2 == 0;
                button.label = fieldValue(f.id) + (blink ? "_" : "");
                button.error = f.id == Field::Ip && ipInvalid;
            });
    }

    void systemServerLost(GameManager &gm)
    {
        bool lost = false;

        gm.view<ClientManager>().each([&](Entity, ClientManager &c) {
            if (c.isTimedOut())
                lost = true;
        });
        if (lost)
            returnToMenu(gm);
    }

    void systemButtons(GameManager &gm)
    {
        // The engine does not expose the cursor position: only raylib call
        // left here. Clicks come from the engine MouseManager.
        const Vector2 mouse = ::GetMousePosition();
        bool pressed = false;
        std::vector<std::function<void(GameManager &)>> clicked;

        gm.view<MouseManager>().each([&pressed](Entity, MouseManager &m) {
            for (const auto button : m.buttonPressed)
                if (button == MouseButton::Value::Left)
                    pressed = true;
        });

        if (activeField != Field::None && pressed) {
            bool onActiveField = false;
            gm.view<TextField, Button>().each(
                [&](Entity, TextField &f, Button &button) {
                    if (f.id == activeField && contains(button.rect, mouse))
                        onActiveField = true;
                });
            if (!onActiveField)
                stopEditing();
        }
        gm.view<Button>().each([&](Entity, Button &button) {
            button.hovered = contains(button.rect, mouse);
            if (button.hovered && pressed && !button.disabled)
                clicked.push_back(button.onClick);
        });
        for (auto &callback : clicked)
            callback(gm);
    }

    void systemButtonVisuals(GameManager &gm)
    {
        std::vector<Entity> orphans;

        gm.view<Button, SpriteSheet2D>().each(
            [](Entity, Button &button, SpriteSheet2D &sheet) {
                if (button.disabled)
                    sheet.currentTile = BUTTON_TILE_DISABLED;
                else if (button.hovered)
                    sheet.currentTile = BUTTON_TILE_HOVERED;
                else
                    sheet.currentTile = BUTTON_TILE_NORMAL;
                sheet.color = button.error ? raylib::Color(255, 110, 110, 255)
                                           : raylib::Color::White();
            });
        gm.view<ButtonLabel, Text2D, Font>().each(
            [&](Entity e, ButtonLabel &label, Text2D &text, Font &font) {
                if (!gm.hasComponent<Button>(label.owner)) {
                    orphans.push_back(e);
                    return;
                }
                const Button &button = gm.getComponent<Button>(label.owner);
                const float maxWidth = button.rect.width - 2.f * BUTTON_TEXT_PADDING;
                float width = 0.f;

                text.str = button.label;
                text.fontSize = LABEL_SIZE;
                width = font.MeasureText(text.str, text.fontSize, text.spacing).x;
                if (width > maxWidth && width > 0.f)
                    text.fontSize = LABEL_SIZE * maxWidth / width;
            });
        for (const Entity e : orphans)
            gm.destroyEntity(e);
    }

    void systemTextAnchor(Text2D &text, Font &font, const TextAnchor &anchor)
    {
        const auto size = font.MeasureText(text.str, text.fontSize, text.spacing);

        text.origin.x = size.x * anchor.x;
        text.origin.y = size.y * anchor.y;
    }

        void systemMenuBackground(GameManager &gm)
    {
        bool hidden = false;
        std::vector<Entity> backgrounds;

        gm.view<LobbyClient>().each([&hidden](Entity, LobbyClient &l) {
            hidden = hidden || l.inGame || l.shown;
        });
        gm.view<MenuBackground>().each(
            [&backgrounds](Entity e, MenuBackground &) { backgrounds.push_back(e); });
        if (hidden) {
            for (const Entity e : backgrounds)
                gm.destroyEntity(e);
            return;
        }
        if (!backgrounds.empty())
            return;
        const Entity e = gm.createEntity();

        gm.addComponent<MenuBackground>(e);
        gm.addComponent<Sprite2D>(e, 1.f);
        gm.addComponent<Texture2D>(
            e, gm.getAssetsMap().at(Asset::MENU_BACKGROUND));
        gm.addComponent<Position2D>(e, 0.f, 0.f);
    }

} // namespace RType::Client
