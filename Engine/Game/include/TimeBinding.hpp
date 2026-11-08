/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TimeBinding
*/

#ifndef TIMEBINDING_HPP_
#define TIMEBINDING_HPP_

#include <sol/sol.hpp>

namespace RType::Game {

    /**
     * @brief Lets scripts read the time of the game's clock, the
     * MasterClock of the Clock module, without exposing the clock itself.
     */
    class TimeBinding {
    public:
        TimeBinding() = delete;

        /**
         * @brief Lets scripts call gm:elapsed(), which gives the time spent
         * during the previous frame by the game's clock, in milliseconds.
         * @throw ModuleNotLoaded If gm:elapsed is called without the Clock
         * module.
         */
        static void registerFunctions(sol::state &lua);
    };

} // namespace RType::Game

#endif /* !TIMEBINDING_HPP_ */
