/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Config
*/

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <array>
#include <cstddef>

namespace RType::Server {

    inline constexpr int WINDOW_WIDTH = 1920;
    inline constexpr int WINDOW_HEIGHT = 1080;
    inline constexpr float SCALE = 4.5f;

    inline constexpr float BACKGROUND_SPEED = 75.f;
    inline constexpr int BACKGROUND_WIDTH = 4224;
    inline constexpr float BACKGROUND_PERIOD = BACKGROUND_WIDTH * SCALE - 2.f;
    inline constexpr float LEVEL_END_X =
        WINDOW_WIDTH - BACKGROUND_WIDTH * SCALE;

    inline constexpr int PLAYER_FRAMES = 5;
    inline constexpr float PLAYER_WIDTH = 166.f / PLAYER_FRAMES * SCALE;
    inline constexpr float PLAYER_HEIGHT = 17.f * SCALE;
    inline constexpr int PLAYER_HP = 1;
    inline constexpr int PLAYER_LIVES = 3;
    inline constexpr float RESPAWN_INVINCIBILITY = 3.f;
    inline constexpr float INVINCIBLE_BLINK = 0.1f;
    inline constexpr unsigned char INVINCIBLE_ALPHA = 70;
    inline constexpr std::size_t PLAYER_COLORS = 5;
    inline constexpr unsigned int PLAYER_FRAME_MS = 200;
    inline constexpr int PLAYER_EXPLOSION_FRAMES = 6;
    inline constexpr float PLAYER_EXPLOSION_WIDTH = 33.f;
    inline constexpr float PLAYER_EXPLOSION_HEIGHT = 30.f;
    inline constexpr unsigned int PLAYER_EXPLOSION_FRAME_MS = 70;
    inline constexpr float PLAYER_DEATH_DURATION = 0.55f;
    inline constexpr float PLAYER_START_X = 100.f;

    inline constexpr std::size_t FRAME_DOWN_MAX = 0;
    inline constexpr std::size_t FRAME_DOWN = 1;
    inline constexpr std::size_t FRAME_NEUTRAL = 2;
    inline constexpr std::size_t FRAME_UP = 3;
    inline constexpr std::size_t FRAME_UP_MAX = 4;

    inline constexpr float TILT_SOFT = 60.f;
    inline constexpr float TILT_HARD = 300.f;

    inline constexpr int BULLET_FRAMES = 3;
    inline constexpr float BULLET_WIDTH = 54.f / BULLET_FRAMES * SCALE;
    inline constexpr float BULLET_HEIGHT = 20.f * SCALE;
    inline constexpr float BULLET_SPEED = 1500.f;
    inline constexpr unsigned int BULLET_FRAME_MS = 60;
    inline constexpr float SHOOT_COOLDOWN = 0.2f;

    inline constexpr float PLAYER_NOSE_Y = 10.f * SCALE;

    inline constexpr float LIFE_ICON_SCALE = 2.f;
    inline constexpr float LIFE_ICON_WIDTH =
        PLAYER_WIDTH / SCALE * LIFE_ICON_SCALE;
    inline constexpr float LIFE_ICON_HEIGHT =
        PLAYER_HEIGHT / SCALE * LIFE_ICON_SCALE;
    inline constexpr float HUD_MARGIN = 20.f;
    inline constexpr float HUD_SPACING = 8.f;

    inline constexpr int CHARGE_FRAMES = 8;
    inline constexpr float CHARGE_SIZE = 32.f * SCALE;
    inline constexpr float CHARGE_OVERLAP = 4.f * SCALE;
    inline constexpr unsigned int CHARGE_FRAME_MS = 60;
    inline constexpr float CHARGE_LEVEL_TIME = 0.35f;
    inline constexpr int CHARGE_MAX_LEVEL = 5;

    inline constexpr int CHARGED_SHOT_FRAMES = 2;
    inline constexpr float CHARGED_SHOT_CELL_WIDTH = 80.f * SCALE;
    inline constexpr float CHARGED_SHOT_CELL_HEIGHT = 16.f * SCALE;
    inline constexpr float CHARGED_SHOT_SPEED = 1500.f;
    inline constexpr unsigned int CHARGED_SHOT_FRAME_MS = 60;

    struct ChargedShotLevel {
        float width;
        float height;
        int damage;
    };

    inline constexpr std::array<ChargedShotLevel, CHARGE_MAX_LEVEL>
        CHARGED_SHOT_LEVELS = {{
            {16.f, 12.f, 2},
            {32.f, 12.f, 3},
            {48.f, 14.f, 5},
            {64.f, 14.f, 8},
            {80.f, 16.f, 12},
        }};

} // namespace RType::Server

#endif
