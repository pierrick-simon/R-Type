/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#ifndef RUNNER_HPP_
#define RUNNER_HPP_

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
         * runs the main loop until GameConfig.runningCondition returns
         * false.
         * @return 0 on success, 84 if the engine raised an error.
         */
        static int run();
    };

} // namespace RType::Game

#endif /* !RUNNER_HPP_ */
