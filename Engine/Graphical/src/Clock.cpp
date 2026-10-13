/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Clock
*/

#include "Clock.hpp"
#include <iostream>

namespace RType {
    void Clock::systemClock(Clock &c)
    {
        auto now = std::chrono::steady_clock::now();
        auto realElapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - c._clock);
        if (realElapsed.count() == 0)
            realElapsed = std::chrono::milliseconds(1);
        c._elapsed = realElapsed * c.speed;
        c._clock = now;
        std::cout << c.getElapsed() << "\n";
    }
}; // namespace RType
