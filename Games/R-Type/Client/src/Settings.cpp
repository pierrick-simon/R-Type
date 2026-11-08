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
#include "Lobby.hpp"
#include "Menu.hpp"
#include "MouseManager.hpp"
#include "Position2D.hpp"
#include "Sound.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        struct FullscreenButton {};

        std::string windowModeLabel()
        {
            return ::IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE) ? "Fullscreen"
                                                                  : "Windowed";
        }

        void changeFullScreen(GameManager &gm)
        {
            ::ToggleBorderlessWindowed();
            gm.view<FullscreenButton, Button>().each(
                [](Entity, FullscreenButton &, Button &button) {
                    button.label = windowModeLabel();
                });
        }

        const std::array<const char *, ACTION_COUNT> ACTION_IDS = {
            "up", "left", "down", "right", "shoot"};

        /** @brief The next input pressed is captured only from the frame
         *         after the click on the key button (not that click). */
        bool captureArmed = false;

        /** @brief One "name value" per line: "volume 7", "up 20", and
         *         "shoot mouse 0" for a mouse button (keys saved as their
         *         Key::Value number, buttons as their MouseButton::Value). */
        void saveSettings()
        {
            std::ofstream file(SETTINGS_PATH, std::ios::trunc);

            file << "volume " << _volumeLevel << '\n';
            for (std::size_t i = 0; i < ACTION_COUNT; i++) {
                const Input &input = _keyBindings.at(i);

                file << ACTION_IDS.at(i) << (input.mouse ? " mouse " : " ")
                     << input.code << '\n';
            }
        }

        void startCapture(Action action)
        {
            if (_waitingKey == action)
                return;
            _waitingKey = action;
            captureArmed = false;
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
            SpriteSheet2D sheet(
                Size2D {1, 3}, BUTTON_TILE_NORMAL, 0, 1.f, false);

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
                const Entity button = addButton(
                    gm,
                    buttonX,
                    y,
                    KeyBindings::inputName(_keyBindings.at(i)),
                    [action](GameManager &) { startCapture(action); },
                    KEY_BUTTON_WIDTH);

                gm.addComponent<SettingsItem>(name);
                gm.addComponent<SettingsItem>(button);
                gm.addComponent<KeyBindButton>(button, KeyBindButton {action});
            }
            const Entity reset = addButton(gm,
                buttonX,
                SETTINGS_FIRST_Y +
                    static_cast<float>(ACTION_COUNT) * KEY_ROW_STEP + 10.f,
                "Reset",
                resetKeys,
                KEY_BUTTON_WIDTH);

            gm.addComponent<SettingsItem>(reset);
        }

        void createSettingsPage(GameManager &gm)
        {
            addSettingsText(
                gm, SETTINGS_TITLE_Y, "Settings", SETTINGS_TITLE_SIZE);
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
            createKeyRows(gm);
            addSettingsButton(gm,
                static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH -
                    BUTTON_GAP * 2,
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
            std::string word;
            int value = 0;
            bool mouse = false;

            if (!(in >> name >> word))
                continue;
            if (word == "mouse") {
                mouse = true;
                if (!(in >> word))
                    continue;
            }
            try {
                value = std::stoi(word);
            } catch (const std::exception &) {
                continue;
            }
            if (name == "volume") {
                _volumeLevel = std::clamp(value, 0, VOLUME_STEPS);
                continue;
            }
            const int count = mouse
                ? static_cast<int>(MouseButton::Value::MouseButtonCount)
                : static_cast<int>(Key::Value::KeyCount);

            if (value < 0 || value >= count)
                continue;
            for (std::size_t i = 0; i < ACTION_COUNT; i++)
                if (name == ACTION_IDS.at(i))
                    KeyBindings::bind(static_cast<Action>(i),
                        mouse ? Input::button(
                                    static_cast<MouseButton::Value>(value))
                              : Input::key(static_cast<Key::Value>(value)));
        }
    }

    void systemKeybind(GameManager &gm, Keybind &bind)
    {
        static std::optional<std::array<Input, ACTION_COUNT>> applied;
        static bool appliedPlaying = false;
        bool playing = false;

        gm.view<LobbyClient>().each([&playing](Entity, LobbyClient &l) {
            playing = playing || (l.inGame && !l.gameOverShown);
        });
        if (applied == _keyBindings && appliedPlaying == playing)
            return;
        if (playing) {
            KeyBindings::fillKeybind(bind);
        } else {
            bind._keyToKey.clear();
            bind._keyToMouseButton.clear();
            bind._mouseButtonToKey.clear();
            bind._mouseButtonToMouseButton.clear();
        }
        applied = _keyBindings;
        appliedPlaying = playing;
    }

    void systemSettings(GameManager &gm)
    {
        if (gm.view<SettingsItem>().getVector().empty())
            return;
        if (_waitingKey && !captureArmed) {
            captureArmed = true;
        } else if (_waitingKey) {
            std::optional<Input> pressed;

            gm.view<KeyboardManager>().each(
                [&pressed](Entity, KeyboardManager &k) {
                    if (!k.keyPressed.empty())
                        pressed = Input::key(k.keyPressed.front());
                });
            gm.view<MouseManager>().each([&pressed](Entity, MouseManager &m) {
                if (!pressed && !m.buttonPressed.empty())
                    pressed = Input::button(m.buttonPressed.front());
            });
            const bool refused = pressed && !pressed->mouse &&
                (pressed->code == static_cast<int>(Key::Value::Escape) ||
                    pressed->code == static_cast<int>(Key::Value::Enter) ||
                    pressed->code == static_cast<int>(Key::Value::Tab) ||
                    pressed->code == static_cast<int>(Key::Value::Backspace));

            if (pressed && !refused) {
                KeyBindings::bind(*_waitingKey, *pressed);
                saveSettings();
            }
            if (pressed)
                _waitingKey.reset();
        }
        gm.view<SettingsItem, Button>().each(
            [&gm](Entity e, SettingsItem &, Button &button) {
                if (gm.hasComponent<KeyBindButton>(e))
                    return;
                if (gm.hasComponent<VolumeButton>(e))
                    button.disabled = _waitingKey.has_value() ||
                        volumeButtonDisabled(
                            gm.getComponent<VolumeButton>(e).step);
                else
                    button.disabled = _waitingKey.has_value();
            });
        gm.view<KeyBindButton, Button>().each(
            [](Entity, KeyBindButton &key, Button &button) {
                button.label = _waitingKey == key.action
                    ? "Press a key"
                    : KeyBindings::inputName(_keyBindings.at(
                          static_cast<std::size_t>(key.action)));
                button.selected = _waitingKey == key.action;
            });
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
