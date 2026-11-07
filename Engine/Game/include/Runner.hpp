/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include <atomic>

namespace RType::Game {

    /**
     * @brief Entry point of the engine: builds the game described by
     * GameConfig and runs it.
     */
    class Runner {
    public:
        Runner() = delete;

        /**
         * @brief Loads the modules, calls the start of every script, then
         * runs the main loop until running is false.
         * @return 0 on success, 84 if the engine raised an error.
         */
        static int run();

        /**
         * @brief Read before each frame: the main loop goes on as long as
         * it is true. Set to false by gm:stop(). Atomic since the console
         * thread reads it too.
         */
        static std::atomic<bool> running;
    };

} // namespace RType::Game

#endif /* !RUNNER_HPP_ */
