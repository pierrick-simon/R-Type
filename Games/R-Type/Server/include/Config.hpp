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

    inline constexpr float BACKGROUND_SPEED = 125.f;
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

    inline constexpr float SCORE_FONT_SIZE = 28.f;
    inline constexpr std::size_t SCORE_DIGITS = 6;
    inline constexpr float SCORE_Y =
        WINDOW_HEIGHT - HUD_MARGIN - SCORE_FONT_SIZE;
    inline constexpr float SCORE_PLAYERS_X = 400.f;
    // The game font is monospaced: a character is as wide as the font size.
    inline constexpr float GAME_OVER_SCORE_SIZE = 64.f;
    inline constexpr float GAME_OVER_SCORE_Y = 450.f;
    inline constexpr float GAME_OVER_PLAYER_SIZE = 36.f;
    inline constexpr float GAME_OVER_PLAYERS_Y = 540.f;
    inline constexpr float GAME_OVER_PLAYER_SPACING = 46.f;
    inline constexpr float SCORE_SPACING = 300.f;
    // Main color of each player sheet.
    inline constexpr std::array<std::array<unsigned char, 3>, PLAYER_COLORS>
        PLAYER_SCORE_COLORS = {{
            {82, 115, 255},
            {255, 132, 107},
            {123, 255, 123},
            {230, 90, 230},
            {0, 231, 222},
        }};

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

    inline constexpr int POWER_UP_FRAMES = 12;
    inline constexpr float POWER_UP_WIDTH = 17.f * SCALE;
    inline constexpr float POWER_UP_HEIGHT = 18.f * SCALE;
    inline constexpr unsigned int POWER_UP_FRAME_MS = 80;
    inline constexpr float POWER_UP_DROP_CHANCE = 0.12f;
    inline constexpr float POWER_UP_LIFETIME = 12.f;

    inline constexpr int FORCE_FRAMES = 6;
    inline constexpr int FORCE_MAX_LEVEL = 2;
    inline constexpr float FORCE_WIDTH = 32.f * SCALE;
    inline constexpr float FORCE_HEIGHT = 24.f * SCALE;
    inline constexpr unsigned int FORCE_FRAME_MS = 80;
    inline constexpr float FORCE_OVERLAP = 6.f * SCALE;
    inline constexpr int FORCE_CONTACT_DAMAGE = 1;
    inline constexpr float FORCE_CONTACT_COOLDOWN = 0.15f;

    // Visible part of the Force inside its 32x24 cell, per level.
    inline constexpr std::array<std::array<float, 4>, FORCE_MAX_LEVEL>
        FORCE_HITBOXES = {{
            {6.f, 4.f, 20.f, 16.f},
            {2.f, 3.f, 27.f, 18.f},
        }};

    // The lasers are drawn smaller than the rest of the game.
    inline constexpr float LASER_SCALE = 2.5f;
    inline constexpr float LASER_COOLDOWN = 0.25f;
    inline constexpr float LASER_SPEED = 1500.f;

    inline constexpr int MUZZLE_FRAMES = 4;
    inline constexpr float MUZZLE_WIDTH = 16.f * LASER_SCALE;
    inline constexpr float MUZZLE_HEIGHT = 18.f * LASER_SCALE;
    inline constexpr unsigned int MUZZLE_FRAME_MS = 40;

    // ReflectLaser.png: one 16x18 cell per piece.
    inline constexpr int REFLECT_FRAMES = 7;
    inline constexpr std::size_t REFLECT_STRAIGHT = 0;
    inline constexpr std::size_t REFLECT_UP = 1; // "/", also going down-left
    inline constexpr std::size_t REFLECT_DOWN =
        2; // backslash, also going up-left
    inline constexpr std::size_t REFLECT_TIP_TOP = 3;
    inline constexpr std::size_t REFLECT_TIP_BOTTOM = 4;
    inline constexpr std::size_t REFLECT_TIP_LEFT = 5;
    inline constexpr std::size_t REFLECT_TIP_RIGHT = 6;
    inline constexpr int REFLECT_MAX_BOUNCES = 8;
    inline constexpr float REFLECT_WIDTH = 16.f * LASER_SCALE;
    inline constexpr float REFLECT_HEIGHT = 18.f * LASER_SCALE;
    inline constexpr float REFLECT_TIP_TIME = 0.06f;
    inline constexpr int REFLECT_DAMAGE = 1;

    // HelixLaser.png: one row of 8 frames of 64x32.
    inline constexpr int HELIX_FRAMES = 8;
    inline constexpr float HELIX_WIDTH = 64.f * LASER_SCALE;
    inline constexpr float HELIX_HEIGHT = 32.f * LASER_SCALE;
    inline constexpr unsigned int HELIX_FRAME_MS = 40;
    inline constexpr std::array<int, FORCE_MAX_LEVEL> HELIX_DAMAGE = {4, 8};

    inline constexpr int RING_FRAMES = 5;
    inline constexpr float RING_WIDTH = 56.f * LASER_SCALE;
    inline constexpr float RING_HEIGHT = 48.f * LASER_SCALE;
    inline constexpr float RING_SPEED = 1100.f;
    inline constexpr unsigned int RING_FRAME_MS = 50;
    inline constexpr float RING_SPREAD = 20.f * LASER_SCALE;
    inline constexpr int RING_DAMAGE = 2;
    // Width and height of the ring drawn in each frame of RingLaser.png.
    inline constexpr std::array<std::array<float, 2>, RING_FRAMES> RING_SIZES =
        {{
            {24.f, 22.f},
            {32.f, 28.f},
            {40.f, 34.f},
            {48.f, 42.f},
            {56.f, 48.f},
        }};

} // namespace RType::Server

#endif
