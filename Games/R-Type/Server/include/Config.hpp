/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Config
*/

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <cstddef>

namespace RType::Server {

    inline constexpr int WINDOW_WIDTH = 1920;
    inline constexpr int WINDOW_HEIGHT = 1080;
    inline constexpr float SCALE = 4.5f;

    inline constexpr float BACKGROUND_SPEED = 250.f;
    inline constexpr int BACKGROUND_WIDTH = 4224;
    inline constexpr float BACKGROUND_PERIOD = BACKGROUND_WIDTH * SCALE - 2.f;

    inline constexpr int PLAYER_FRAMES = 5;
    inline constexpr float PLAYER_WIDTH = 166.f / PLAYER_FRAMES * SCALE;
    inline constexpr float PLAYER_HEIGHT = 17.f * SCALE;
    inline constexpr float PLAYER_ACCELERATION = 3000.f;
    inline constexpr float PLAYER_FRICTION = 120.f;
    inline constexpr int PLAYER_HP = 3;
    inline constexpr unsigned int PLAYER_FRAME_MS = 200;

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
    inline constexpr unsigned int BULLET_FRAME_MS = 15;
    inline constexpr float SHOOT_COOLDOWN = 0.2f;

    inline constexpr const char *BACKGROUND_PATH = "R-Type/sprites/Levels/Level1_Background.png";
    inline constexpr const char *PLAYER_PATH = "R-Type/sprites/PlayerSheets/r-typePlayerSheet1.gif";
    inline constexpr const char *BULLET_PATH = "R-Type/sprites/PlayerSheets/PlayerBullet.gif";

}

#endif
