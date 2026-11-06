/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Settings
*/

#include "Settings.hpp"

#include <vector>

#include "Client.hpp"
#include "Menu.hpp"

namespace RType::Client {

    namespace {

        struct FullscreenButton {};

        std::string windowModeLabel()
        {
            return ::IsWindowFullscreen() ? "Fullscreen" : "Windowed";
        }

        void changeFullScreen(GameManager &gm)
        {
            ::ToggleFullscreen();
            gm.view<FullscreenButton, Button>().each(
                [](Entity, FullscreenButton &, Button &button) {
                    button.label = windowModeLabel();
                });
        }

        template<typename Tag>
        void destroyTagged(GameManager &gm)
        {
            std::vector<Entity> items;

            gm.view<Tag>().each(
                [&items](Entity e, Tag &) { items.push_back(e); });
            for (const Entity e : items)
                gm.destroyEntity(e);
        }

        Entity addSettingsButton(GameManager &gm, float x, float y,
            const std::string &label,
            std::function<void(GameManager &)> onClick)
        {
            const Entity e = addButton(gm, x, y, label, std::move(onClick));

            gm.addComponent<SettingsItem>(e);
            return e;
        }

        Entity addSettingsText(
            GameManager &gm, float y, const std::string &str, float size)
        {
            const Entity e = addText(
                gm, static_cast<float>(WINDOW_WIDTH) / 2.f, y, str, size);

            gm.addComponent<SettingsItem>(e);
            return e;
        }

        void createSettingsPage(GameManager &gm)
        {
            const float centerX =
                (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f;

            addSettingsText(gm, SETTINGS_TITLE_Y, "Settings", SETTINGS_TITLE_SIZE);
            addSettingsText(
                gm, SETTINGS_FIRST_Y - BUTTON_GAP / 2.f, "Window Mode", 25.f);
            const Entity mode = addSettingsButton(gm,
                centerX,
                SETTINGS_FIRST_Y,
                windowModeLabel(),
                changeFullScreen);

            gm.addComponent<FullscreenButton>(mode);
            addSettingsButton(gm,
                static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH - BUTTON_GAP * 2,
                static_cast<float>(WINDOW_HEIGTH) - BUTTON_HEIGHT * 2 -
                    BUTTON_GAP,
                "Back",
                closeSettings);
        }

    } // namespace

    void openSettings(GameManager &gm)
    {
        activeField = Field::None;
        destroyTagged<MenuItem>(gm);
        createSettingsPage(gm);
    }

    void closeSettings(GameManager &gm)
    {
        destroyTagged<SettingsItem>(gm);
        createMenu(gm);
    }

} // namespace RType::Client
