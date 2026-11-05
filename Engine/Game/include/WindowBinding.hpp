/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WindowBinding
*/

#ifndef WINDOWBINDING_HPP_
#define WINDOWBINDING_HPP_

#include <sol/sol.hpp>

namespace RType::Game {

    /**
     * @brief Lets scripts know when the window of the Window module is
     * asked to close, without exposing the window itself.
     */
    class WindowBinding {
    public:
        WindowBinding() = delete;

        /**
         * @brief Lets scripts call gm:windowShouldClose(), which tells
         * whether the window's close button, or Escape, was pressed.
         * @throw ModuleNotLoaded If gm:windowShouldClose is called without
         * the Window module.
         */
        static void registerFunctions(sol::state &lua);
    };

} // namespace RType::Game

#endif /* !WINDOWBINDING_HPP_ */
