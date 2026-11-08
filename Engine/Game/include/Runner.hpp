/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#ifndef RUNNER_HPP_
#define RUNNER_HPP_

#include "GameManager.hpp"

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
         * it is true. Set to false by gm:stop().
         */
        static bool running;

    private:
        /**
         * @brief Destroys the entities that hold a Texture2D, a Model3D or
         * a Font while the window still exists: raylib needs it to free them,
         * and the GameManager destroys its components in no given order.
         */
        static void unloadGraphics(GameManager &gm);
    };

} // namespace RType::Game

#endif /* !RUNNER_HPP_ */
