/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_

#include <cstddef>
#include <functional>
#include <list>
#include <string>

namespace RType {

    /**
     * @brief What the game gives the engine: how to run it, its window
     * and the modules it needs. Defined by the game as GameConfig.
     */
    struct GameInfo {
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
        std::string srcDir = ".";
        std::list<std::string> modules;
    };

    extern GameInfo GameConfig;

} // namespace RType

#endif /* !GAME_HPP_ */
