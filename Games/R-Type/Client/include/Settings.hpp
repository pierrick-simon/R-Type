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

    struct SettingsItem {};

    void openSettings(GameManager &gm);
    void closeSettings(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_SETTINGS_HPP_ */
