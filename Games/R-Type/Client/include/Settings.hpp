/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Settings
*/

#ifndef CLIENT_SETTINGS_HPP_
#define CLIENT_SETTINGS_HPP_

#include <optional>
#include "GameManager.hpp"
#include "KeyBindings.hpp"

namespace RType::Client {

    inline constexpr float SETTINGS_TITLE_Y = 200.f;
    inline constexpr float SETTINGS_TITLE_SIZE = 64.f;
    /** @brief Top of the first setting (its label is drawn above it). */
    inline constexpr float SETTINGS_FIRST_Y = 380.f;
    /** @brief Two columns: display and sound on the left, keys on the
     *         right. */
    inline constexpr float SETTINGS_LEFT_X = 560.f;
    inline constexpr float SETTINGS_RIGHT_X = 1360.f;
    /** @brief Key buttons: narrower than the other buttons (5 rows). */
    inline constexpr float KEY_BUTTON_WIDTH = 260.f;
    inline constexpr float KEY_ROW_STEP = 75.f;

    /** @brief Client file of the settings: volume and keys. */
    inline constexpr const char *SETTINGS_PATH = "settings.txt";

    /** @brief Top of the volume row (bar + its two buttons). */
    inline constexpr float SETTINGS_VOLUME_Y = 560.f;
    /** @brief Volume bar: 10 slices of 10 %, same size as a button. */
    inline constexpr int VOLUME_STEPS = 10;
    inline constexpr float VOLUME_BAR_WIDTH = 400.f;
    inline constexpr float VOLUME_BUTTON_SIZE = 90.f;
    inline constexpr float VOLUME_BUTTON_GAP = 30.f;

    /** @brief Every entity of the Settings page. */
    struct SettingsItem {};

    /** @brief Key button of an action. */
    struct KeyBindButton {
        Action action;
    };

    /** @brief Action waiting for its new key (after a click on its
     *         button), nothing otherwise. */
    inline std::optional<Action> _waitingKey;

    /** @brief Volume buttons: @p step is -1 (left) or +1 (right). */
    struct VolumeButton {
        int step;
    };

    /**
     * @brief Master volume, from 0 (muted) to VOLUME_STEPS (100 %).
     *
     * Applied to every Sound component by systemVolume, with
     * Sound::setVolume (miniaudio itself is not exported by engine-audio).
     */
    inline int _volumeLevel = VOLUME_STEPS;

    /** @brief "Settings" button of the main menu: menu -> Settings page. */
    void openSettings(GameManager &gm);
    /** @brief "Back" button of the Settings page: Settings page -> menu. */
    void closeSettings(GameManager &gm);

    /** @brief At launch: reads SETTINGS_PATH (volume and keys). */
    void loadSettings();

    /** @brief Settings page: gets the new key of an action, updates the
     *         labels of the key buttons. */
    void systemSettings(GameManager &gm);

    /**
     * @brief Keep the engine Keybind in sync with the bindings.
     *
     * Refills the Keybind when the bindings change. It is only filled while
     * the player controls the ship (in game, not on the game over screen):
     * everywhere else it stays empty, so the menus get the real keys and
     * clicks (a click bound to "shoot" must still press the buttons).
     *
     * @warning Must run before Keybind::systemUpdateKeybind.
     */
    void systemKeybind(GameManager &gm, Keybind &bind);

    /** @brief Applies _volumeLevel to every Sound (music and effects). */
    void systemVolume(GameManager &gm);

    /** @brief Draws the volume bar of the Settings page (raylib). Must run
     *         after the page background (Sprite2D). */
    void systemDrawSettings(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_SETTINGS_HPP_ */
