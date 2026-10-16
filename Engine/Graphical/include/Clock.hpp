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

    /**
     * @brief Component measuring the time elapsed between two updates.
     *
     * The clock is refreshed by calling systemClock() once per frame. Other
     * systems (e.g. SpriteSheet2D::systemSpriteSheet2D) read the elapsed time
     * through getElapsed().
     */
    class Clock {
    public:
        std::size_t speed = 1;

        [[nodiscard]] std::chrono::milliseconds getElapsed() const
        {
            return _elapsed;
        }

        /**
         * @brief System updating a clock.
         *
         * Computes the time elapsed since the previous call and stores it so
         * that it can be retrieved with getElapsed().
         *
         * @param c The clock to update.
         *
         * @warning Must be called exactly once per frame, otherwise the
         *          elapsed time will not match the frame duration.
         */
        static void systemClock(Clock &c);

    private:
        std::chrono::steady_clock::time_point _clock =
            std::chrono::steady_clock::now();
        std::chrono::milliseconds _elapsed = std::chrono::milliseconds(0);
    };

}; // namespace RType

#endif /* !CLOCK_HPP_ */
