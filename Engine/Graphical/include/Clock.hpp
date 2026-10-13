/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Clock
*/

#ifndef CLOCK_HPP_
#define CLOCK_HPP_

#include <chrono>

namespace RType {

    class Clock {
    public:
        std::size_t speed = 1;

        [[nodiscard]] std::chrono::milliseconds getElapsed() const
        {
            return _elapsed;
        }

        static void systemClock(Clock &c);

    private:
        std::chrono::steady_clock::time_point _clock =
            std::chrono::steady_clock::now();
        std::chrono::milliseconds _elapsed = std::chrono::milliseconds(0);
    };

}; // namespace RType

#endif /* !CLOCK_HPP_ */
