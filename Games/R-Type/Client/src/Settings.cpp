/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Settings
*/

#include "Settings.hpp"

#include <algorithm>
#include <vector>

#include "Assets.hpp"
#include "Client.hpp"
#include "Menu.hpp"
#include "Position2D.hpp"
#include "Sound.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

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

        float volumeBarX()
        {
            return (static_cast<float>(WINDOW_WIDTH) - VOLUME_BAR_WIDTH) / 2.f;
        }

        bool volumeButtonDisabled(const int step)
        {
            return (step < 0 && _volumeLevel <= 0) ||
                (step > 0 && _volumeLevel >= VOLUME_STEPS);
        }

        void changeVolume(GameManager &gm, const int step)
        {
            _volumeLevel = std::clamp(_volumeLevel + step, 0, VOLUME_STEPS);
            gm.view<VolumeButton, Button>().each(
                [](Entity, VolumeButton &volume, Button &button) {
                    button.disabled = volumeButtonDisabled(volume.step);
                });
        }

        Entity addVolumeButton(
            GameManager &gm, float x, float y, const int step, AssetId asset)
        {
            const Entity e = gm.createEntity();

            gm.addComponent<Button>(e,
                Button {{x, y, VOLUME_BUTTON_SIZE, VOLUME_BUTTON_SIZE},
                    "",
                    [step](GameManager &g) { changeVolume(g, step); },
                    false,
                    false,
                    volumeButtonDisabled(step)});
            gm.addComponent<Position2D>(e, x, y);
            gm.addComponent<SpriteSheet2D>(
                e, Size2D {1, 3}, BUTTON_TILE_NORMAL, 0, 1.f, false);
            gm.addComponent<Texture2D>(e, gm.getAssetsMap().at(asset));
            gm.addComponent<VolumeButton>(e, VolumeButton {step});
            gm.addComponent<SettingsItem>(e);
            return e;
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
            addSettingsText(
                gm, SETTINGS_VOLUME_Y - BUTTON_GAP / 2.f, "Volume", 25.f);
            addVolumeButton(gm,
                volumeBarX() - VOLUME_BUTTON_GAP - VOLUME_BUTTON_SIZE,
                SETTINGS_VOLUME_Y,
                -1,
                Asset::MENU_VOLUME_DOWN);
            addVolumeButton(gm,
                volumeBarX() + VOLUME_BAR_WIDTH + VOLUME_BUTTON_GAP,
                SETTINGS_VOLUME_Y,
                1,
                Asset::MENU_VOLUME_UP);
            // Bottom right, like the other "Back" buttons.
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

    void systemVolume(GameManager &gm)
    {
        const float volume =
            static_cast<float>(_volumeLevel) / static_cast<float>(VOLUME_STEPS);

        gm.view<Sound>().each(
            [volume](Entity, Sound &sound) { sound.setVolume(volume); });
    }

    void systemDrawSettings(GameManager &gm)
    {
        if (gm.view<SettingsItem>().getVector().empty())
            return;
        const float x = volumeBarX();
        const float y = SETTINGS_VOLUME_Y;
        const float pad = 8.f;
        const float gap = 4.f;
        const float slice = (VOLUME_BAR_WIDTH - 2.f * pad -
                                gap * static_cast<float>(VOLUME_STEPS - 1)) /
            static_cast<float>(VOLUME_STEPS);
        const Rectangle frame {x, y, VOLUME_BAR_WIDTH, BUTTON_HEIGHT};

        // Same look as the buttons: dark panel, thin border.
        ::DrawRectangleRec(frame, Color {16, 22, 30, 200});
        ::DrawRectangleLinesEx(frame, 1.f, Color {170, 195, 215, 90});
        for (int i = 0; i < VOLUME_STEPS; i++) {
            const Rectangle r {x + pad + static_cast<float>(i) * (slice + gap),
                y + pad,
                slice,
                BUTTON_HEIGHT - 2.f * pad};

            if (i < _volumeLevel)
                ::DrawRectangleRec(r, Color {150, 225, 255, 230});
            else
                ::DrawRectangleLinesEx(r, 1.f, Color {120, 140, 160, 120});
        }
    }

} // namespace RType::Client
