/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Settings
*/

#ifndef CLIENT_SETTINGS_HPP_
#define CLIENT_SETTINGS_HPP_

#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr float SETTINGS_TITLE_Y = 200.f;
    inline constexpr float SETTINGS_TITLE_SIZE = 64.f;
    inline constexpr float SETTINGS_FIRST_Y = 380.f;

    /** @brief Top  the volume row (bar + its two buttons). */
    inline constexpr float SETTINGS_VOLUME_Y = 560.f;
    /** @brief Volume bar: 10 slices of 10 %, same size as a button. */
    inline constexpr int VOLUME_STEPS = 10;
    inline constexpr float VOLUME_BAR_WIDTH = 400.f;
    inline constexpr float VOLUME_BUTTON_SIZE = 90.f;
    inline constexpr float VOLUME_BUTTON_GAP = 30.f;

    /** @brief Every entity of the Settings page. */
    struct SettingsItem {};

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

    /** @brief Applies _volumeLevel to every Sound (music and effects). */
    void systemVolume(GameManager &gm);

    /** @brief Draws the volume bar of the Settings page (raylib). Must run
     *         after the page background (Sprite2D). */
    void systemDrawSettings(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_SETTINGS_HPP_ */
