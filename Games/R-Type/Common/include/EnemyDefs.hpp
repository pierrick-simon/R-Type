/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyDefs
*/

#ifndef ENEMYDEFS_HPP_
#define ENEMYDEFS_HPP_

#include <array>
#include <cstddef>

#include "GameComponents.hpp"
#include "generated/EnemySheets.hpp"

namespace RType::Game {

    /// Display scale of the sprites (the originals are tiny, ~30 px).
    constexpr float SPRITE_SCALE = 3.f;

    /// Part of the visible sprite that really hurts (< 1 = forgiving).
    constexpr float HITBOX_RATIO = 0.8f;

    /**
     * @brief Static description of an enemy kind: its stats and its look.
     *
     * This is data, not an entity: EnemyFactory::spawn() reads it to build
     * the components of a new enemy.
     */
    struct EnemyDef {
        const char *name;
        const Assets::SheetInfo *sheet; ///< Sprite strip (client side).
        int hp;
        int scoreValue;
        int contactDamage;
        PatternType pattern;
        float speed; ///< px/s to the left.
        float amplitude; ///< px (SineWave / ZigZag only).
        float period; ///< s (SineWave / ZigZag only).
    };

    // clang-format off
    inline constexpr std::array<EnemyDef, std::size_t(EnemyType::Count)>
        ENEMY_DEFS = {{
        // name           sheet                   hp  score dmg pattern                 speed amp   period
        {"scout",         &Assets::SCOUT,          1,  100,  1, PatternType::Straight,  320.f,   0.f, 1.0f},
        {"spike_orb",     &Assets::SPIKE_ORB,      2,  150,  1, PatternType::SineWave,  220.f, 120.f, 2.0f},
        {"armored_eye",   &Assets::ARMORED_EYE,    4,  300,  1, PatternType::ZigZag,    180.f, 100.f, 2.5f},
        {"pod_spinner",   &Assets::POD_SPINNER,    6,  400,  2, PatternType::Straight,  120.f,   0.f, 1.0f},
        {"twin_ship",     &Assets::TWIN_SHIP,      3,  200,  1, PatternType::SineWave,  260.f,  80.f, 1.5f},
        {"walker",        &Assets::WALKER,         5,  250,  1, PatternType::Straight,   90.f,   0.f, 1.0f},
        {"gunship_cyan",  &Assets::GUNSHIP_CYAN,   2,  120,  1, PatternType::Straight,  240.f,   0.f, 1.0f},
        {"gunship_pink",  &Assets::GUNSHIP_PINK,   3,  180,  1, PatternType::SineWave,  240.f,  60.f, 1.2f},
        {"gunship_green", &Assets::GUNSHIP_GREEN,  4,  240,  1, PatternType::ZigZag,    260.f,  70.f, 1.6f},
        {"gunship_red",   &Assets::GUNSHIP_RED,    5,  320,  2, PatternType::SineWave,  300.f, 110.f, 1.4f},
        {"gunship_blue",  &Assets::GUNSHIP_BLUE,   6,  400,  2, PatternType::ZigZag,    320.f, 130.f, 1.8f},
    }};
    // clang-format on

    /**
     * @brief Get the description of an enemy kind.
     */
    [[nodiscard]] constexpr const EnemyDef &enemyDef(EnemyType type)
    {
        return ENEMY_DEFS[std::size_t(type)];
    }

} // namespace RType::Game

#endif /* !ENEMYDEFS_HPP_ */
