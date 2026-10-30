/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MenuSystems
*/

#include <vector>

#include "ClientManager.hpp"
#include "Menu.hpp"
#include "Text2D.hpp"
#include "Assets.hpp"
#include "Clock.hpp"

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

    void systemButtons(GameManager &gm)
    {
        const Vector2 mouse = ::GetMousePosition();
        const bool pressed = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        std::vector<std::function<void(GameManager &)>> clicked;

        if (activeField != Field::None && pressed) {
            bool onActiveField = false;
            gm.view<TextField, Button>().each(
                [&](Entity, TextField &f, Button &button) {
                    if (f.id == activeField && ::CheckCollisionPointRec(mouse, button.rect))
                        onActiveField = true;
                });
            if (!onActiveField)
                stopEditing();
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
            Color fill = button.hovered ? Color {70, 120, 220, 255} : Color {40, 50, 90, 255};
            if (button.error)
                fill = button.hovered ? Color {200, 70, 70, 255} : Color {150, 40, 40, 255};
            const int textWidth = ::MeasureText(button.label.c_str(), LABEL_SIZE);
            ::DrawRectangleRec(button.rect, fill);
            ::DrawRectangleLinesEx(button.rect, 3.f, WHITE);
            ::DrawText(button.label.c_str(), static_cast<int>(button.rect.x + (button.rect.width - textWidth) / 2),
                static_cast<int>(button.rect.y + (button.rect.height - LABEL_SIZE) / 2),
                LABEL_SIZE, WHITE);
        });
    }

} // namespace RType::Client
