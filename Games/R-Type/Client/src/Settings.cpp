/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Settings
*/

#include "Settings.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

#include "Assets.hpp"
#include "Client.hpp"
#include "KeyboardManager.hpp"
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

        const std::array<const char *, ACTION_COUNT> ACTION_IDS = {
            "up", "left", "down", "right", "shoot"};

        /** @brief One "name value" per line: "volume 7", "up 20"... (keys
         *         saved as their Key::Value number). */
        void saveSettings()
        {
            std::ofstream file(SETTINGS_PATH, std::ios::trunc);

            file << "volume " << _volumeLevel << '\n';
            for (std::size_t i = 0; i < ACTION_COUNT; i++)
                file << ACTION_IDS.at(i) << ' '
                     << static_cast<int>(_keyBindings.at(i)) << '\n';
        }

        float volumeBarX()
        {
            return SETTINGS_LEFT_X - VOLUME_BAR_WIDTH / 2.f;
        }

        /** @brief The left button is greyed at 0 %, the right one at 100 %. */
        bool volumeButtonDisabled(const int step)
        {
            return (step < 0 && _volumeLevel <= 0) ||
                (step > 0 && _volumeLevel >= VOLUME_STEPS);
        }

        void changeVolume(GameManager &gm, const int step)
        {
            _volumeLevel = std::clamp(_volumeLevel + step, 0, VOLUME_STEPS);
            saveSettings();
            gm.view<VolumeButton, Button>().each(
                [](Entity, VolumeButton &volume, Button &button) {
                    button.disabled = volumeButtonDisabled(volume.step);
                });
        }

        /** @brief Square button (90 x 90 sprite with its arrow, no label). */
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
            SpriteSheet2D sheet(Size2D {1, 3}, BUTTON_TILE_NORMAL, 0, 1.f, false);

            sheet.assetId = asset;
            gm.addComponent<SpriteSheet2D>(e, sheet);
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

        void resetKeys(GameManager &)
        {
            _keyBindings = DEFAULT_KEYS;
            _waitingKey.reset();
            saveSettings();
        }

        /** @brief Right column: one row per action (name + key button),
         *         then Reset. */
        void createKeyRows(GameManager &gm)
        {
            const float buttonX = SETTINGS_RIGHT_X - 20.f;
            const float buttonHeight =
                BUTTON_HEIGHT * KEY_BUTTON_WIDTH / BUTTON_WIDTH;

            const Entity title = addText(gm,
                buttonX + KEY_BUTTON_WIDTH / 2.f,
                SETTINGS_FIRST_Y - BUTTON_GAP / 2.f,
                "Controls",
                25.f);

            gm.addComponent<SettingsItem>(title);
            for (std::size_t i = 0; i < ACTION_COUNT; i++) {
                const auto action = static_cast<Action>(i);
                const float y =
                    SETTINGS_FIRST_Y + static_cast<float>(i) * KEY_ROW_STEP;
                const Entity name = addText(gm,
                    SETTINGS_RIGHT_X - 50.f,
                    y + buttonHeight / 2.f,
                    KeyBindings::actionName(action),
                    30.f,
                    TextAnchor {1.f, 0.5f});
                const Entity button = addButton(gm,
                    buttonX,
                    y,
                    KeyBindings::keyName(_keyBindings.at(i)),
                    [action](GameManager &) { _waitingKey = action; },
                    KEY_BUTTON_WIDTH);

                gm.addComponent<SettingsItem>(name);
                gm.addComponent<SettingsItem>(button);
                gm.addComponent<KeyBindButton>(button, KeyBindButton {action});
            }
            const Entity reset = addButton(gm,
                buttonX,
                SETTINGS_FIRST_Y + static_cast<float>(ACTION_COUNT) * KEY_ROW_STEP +
                    10.f,
                "Reset",
                resetKeys,
                KEY_BUTTON_WIDTH);

            gm.addComponent<SettingsItem>(reset);
        }

        void createSettingsPage(GameManager &gm)
        {
            addSettingsText(gm, SETTINGS_TITLE_Y, "Settings", SETTINGS_TITLE_SIZE);
            // Left column: display and sound.
            const Entity modeText = addText(gm,
                SETTINGS_LEFT_X,
                SETTINGS_FIRST_Y - BUTTON_GAP / 2.f,
                "Window Mode",
                25.f);
            const Entity mode = addSettingsButton(gm,
                SETTINGS_LEFT_X - BUTTON_WIDTH / 2.f,
                SETTINGS_FIRST_Y,
                windowModeLabel(),
                changeFullScreen);

            gm.addComponent<SettingsItem>(modeText);
            gm.addComponent<FullscreenButton>(mode);
            const Entity volumeText = addText(gm,
                SETTINGS_LEFT_X,
                SETTINGS_VOLUME_Y - BUTTON_GAP / 2.f,
                "Volume",
                25.f);

            gm.addComponent<SettingsItem>(volumeText);
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
            // Right column: the keys.
            createKeyRows(gm);
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
        _waitingKey.reset();
        activeField = Field::None;
        destroyTagged<MenuItem>(gm);
        createSettingsPage(gm);
    }

    void closeSettings(GameManager &gm)
    {
        _waitingKey.reset();
        destroyTagged<SettingsItem>(gm);
        createMenu(gm);
    }

    void loadSettings()
    {
        std::ifstream file(SETTINGS_PATH);
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream in(line);
            std::string name;
            int value = 0;

            if (!(in >> name >> value))
                continue;
            if (name == "volume") {
                _volumeLevel = std::clamp(value, 0, VOLUME_STEPS);
                continue;
            }
            for (std::size_t i = 0; i < ACTION_COUNT; i++)
                if (name == ACTION_IDS.at(i) && value >= 0 &&
                    value < static_cast<int>(Key::Value::KeyCount))
                    KeyBindings::bind(
                        static_cast<Action>(i), static_cast<Key::Value>(value));
        }
    }

    void systemSettings(GameManager &gm)
    {
        if (gm.view<SettingsItem>().getVector().empty())
            return;
        if (_waitingKey) {
            std::optional<Key::Value> pressed;

            gm.view<KeyboardManager>().each([&pressed](Entity, KeyboardManager &k) {
                if (!k.keyPressed.empty())
                    pressed = k.keyPressed.front();
            });
            // Escape cancels; Enter, Tab and Backspace stay for the menus.
            if (pressed && *pressed != Key::Value::Escape &&
                *pressed != Key::Value::Enter && *pressed != Key::Value::Tab &&
                *pressed != Key::Value::Backspace) {
                KeyBindings::bind(*_waitingKey, *pressed);
                saveSettings();
            }
            if (pressed)
                _waitingKey.reset();
        }
        gm.view<KeyBindButton, Button>().each(
            [](Entity, KeyBindButton &key, Button &button) {
                button.label = _waitingKey == key.action
                    ? "Press a key"
                    : KeyBindings::keyName(
                          _keyBindings.at(static_cast<std::size_t>(key.action)));
                button.selected = _waitingKey == key.action;
            });
    }

    void systemVolume(GameManager &gm)
    {
        const float volume =
            static_cast<float>(_volumeLevel) / static_cast<float>(VOLUME_STEPS);

        // Every sound, every frame: the sounds created during the game
        // (shots, explosions, music) get the volume too.
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
