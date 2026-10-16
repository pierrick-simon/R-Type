/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Components
*/

#ifndef POC_COMPONENTS_HPP_
#define POC_COMPONENTS_HPP_

#include <memory>
#include <string>

#include "Entity.hpp"

namespace Poc {

    struct Position {
        float x;
        float y;
    };

    struct Velocity {
        float x;
        float y;
    };

    struct Health {
        int hp;
    };

    struct Damage {
        int value;
    };

    struct Name {
        std::string value;
    };

    struct Player {};

    struct Enemy {};

    struct Bullet {};

    struct Owned {
        RType::Entity owner;
        int value;
    };

    struct Body {
        float x;
        float y;
        float vx;
        float vy;
    };

    struct Lifetime {
        RType::Entity owner;
        int ticks;
    };

    struct MoveOnly {
        std::unique_ptr<int> ptr;
    };

    struct Tracked {
        static inline int alive = 0;

        int value;

        explicit Tracked(int v = 0) :
            value(v)
        {
            alive++;
        }

        Tracked(const Tracked &other) :
            value(other.value)
        {
            alive++;
        }

        Tracked(Tracked &&other) noexcept :
            value(other.value)
        {
            alive++;
        }

        Tracked &operator=(const Tracked &) = default;
        Tracked &operator=(Tracked &&) noexcept = default;

        ~Tracked()
        {
            alive--;
        }
    };

} // namespace Poc

#endif /* !POC_COMPONENTS_HPP_ */
