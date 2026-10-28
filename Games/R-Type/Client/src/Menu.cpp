/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#include "Menu.hpp"
#include <vector>
#include "ClientManager.hpp"

namespace RType::Client {

    namespace {

        constexpr float BUTTON_WIDTH = 400.f;
        constexpr float BUTTON_HEIGHT = 90.f;
        constexpr float BUTTON_GAP = 40.f;
        constexpr int TITLE_SIZE = 120;
        constexpr int LABEL_SIZE = 48;

        void addButton(GameManager &gm, float y, const std::string &label,
            std::function<void(GameManager &)> onClick)
        {
            const Entity e = gm.createEntity();
            const float x = (1920.f - BUTTON_WIDTH) / 2.f;

            gm.addComponent<MenuItem>(e);
            gm.addComponent<Button>(e,
                Button {{x, y, BUTTON_WIDTH, BUTTON_HEIGHT},
                    label,
                    std::move(onClick),
                    false});
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
            connectToServer(gm);
        }

    } // namespace

    void createMenu(GameManager &gm)
    {
        const float firstY = 480.f;

        addButton(gm, firstY, "Jouer", startGame);
        addButton(gm,
            firstY + BUTTON_HEIGHT + BUTTON_GAP,
            "Quitter",
            [](GameManager &) { quitRequested = true; });
    }

    void connectToServer(GameManager &gm)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, "127.0.0.1", 12345);
    }

    void systemButtons(GameManager &gm)
    {
        const Vector2 mouse = ::GetMousePosition();
        const bool pressed = ::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        std::vector<std::function<void(GameManager &)>> clicked;

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
