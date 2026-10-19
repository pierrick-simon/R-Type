/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameComponents
*/

#ifndef GAMECOMPONENTS_HPP_
#define GAMECOMPONENTS_HPP_

#include <cmath>
#include <cstdint>
#include <numbers>

namespace RType::Game {

    /**
     * @brief Every kind of enemy of the game.
     *
     * @warning The order must match ENEMY_DEFS (see EnemyDefs.hpp): the enum
     *          value is used as the index in that table.
     */
    enum class EnemyType : std::uint8_t {
        Scout,
        SpikeOrb,
        ArmoredEye,
        PodSpinner,
        TwinShip,
        Walker,
        GunshipCyan,
        GunshipPink,
        GunshipGreen,
        GunshipRed,
        GunshipBlue,
        Count
    };

    /**
     * @brief Tag + gameplay data shared by all the enemies.
     */
    struct Enemy {
        EnemyType type;
        int scoreValue;
        int contactDamage;
    };

    /**
     * @brief Hit points of an entity (enemy, player...).
     */
    struct Health {
        int hp;
        int maxHp;
    };

    /**
     * @brief Axis-aligned collision box, in pixels, centered on Position2D.
     */
    struct Hitbox {
        float width;
        float height;
    };

    /**
     * @brief How an enemy moves. All the patterns go to the left.
     */
    enum class PatternType : std::uint8_t {
        Straight, ///< Constant speed, no vertical movement.
        SineWave, ///< Smooth up and down oscillation.
        ZigZag ///< Straight diagonal segments, sharp turns.
    };

    /**
     * @brief Movement of an enemy (data only, see systemEnemyPattern).
     *
     * The vertical oscillation is centered on the spawn height.
     */
    struct MovementPattern {
        PatternType type = PatternType::Straight;
        float speed = 0.f; ///< Horizontal speed to the left, in px/s.
        float amplitude = 0.f; ///< Vertical amplitude, in px.
        float period = 1.f; ///< Duration of one oscillation, in s.
        float time = 0.f; ///< Time elapsed since the spawn, in s.

        /**
         * @brief Vertical velocity (px/s) to apply at the current #time.
         *
         * It is the derivative of the wanted trajectory, so integrating it
         * with Velocity2D gives y0 + amplitude * wave(time).
         */
        [[nodiscard]] float velocityY() const
        {
            if (period <= 0.f)
                return 0.f;
            switch (type) {
                case PatternType::SineWave: {
                    float w = 2.f * std::numbers::pi_v<float> / period;
                    return amplitude * w * std::cos(w * time);
                }
                case PatternType::ZigZag: {
                    float phase = time + period / 4.f;
                    phase -= period * float(int(phase / period));
                    float slope = 4.f * amplitude / period;
                    return phase < period / 2.f ? slope : -slope;
                }
                case PatternType::Straight:
                default:
                    return 0.f;
            }
        }
    };

} // namespace RType::Game

#endif /* !GAMECOMPONENTS_HPP_ */
