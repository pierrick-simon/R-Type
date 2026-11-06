/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_

#include <cstddef>
#include <list>
#include <string>

namespace RType {

    /**
     * @brief What the game gives the engine: its window, its scripts and
     * the modules it needs. Defined by the engine as GameConfig, filled
     * from the game's config.conf by ConfigBuilder.
     */
    struct GameInfo {
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
