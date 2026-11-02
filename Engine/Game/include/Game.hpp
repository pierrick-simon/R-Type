/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_

#include <functional>
#include <list>
#include <string>

#include "GameManager.hpp"

namespace RType {

    struct GameInfo {
        std::list<std::function<void(RType::GameManager &)>> inits = {};
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
    };

    extern GameInfo Game;

} // namespace RType

#endif /* !GAME_HPP_ */
