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
     * asked to close, and read and change its settings, without exposing
     * the window itself.
     */
    class WindowBinding {
    public:
        WindowBinding() = delete;

        /**
         * @brief Lets scripts call:
         * - gm:shouldClose(), which tells whether the window's close
         *   button, or Escape, was pressed;
         * - gm:fps() and gm:setFps(n), for the window's frames per second;
         * - gm:fullScreen() and gm:setFullScreen(b), for full screen;
         * - gm:hideCursor() and gm:setHideCursor(b), for hiding the mouse
         *   cursor and keeping it in the window.
         * A change is applied at the start of the next frame.
         * @throw ModuleNotLoaded If one of these functions is called without
         * the Window module.
         */
        static void registerFunctions(sol::state &lua);
    };

} // namespace RType::Game

#endif /* !WINDOWBINDING_HPP_ */
