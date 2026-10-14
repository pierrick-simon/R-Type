/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Window
*/

#include "Window.hpp"
#include "Exception.hpp"

namespace RType {
    Window::Window(int width, int height, const std::string &name,
        raylib::Color clearScreenColor) :
        clearScreenColor(clearScreenColor)
    {
        static bool onlyOne = true;
        if (!onlyOne)
            throw EngineException("Window can be created only once");
        try {
            this->Init(width, height, name, LOG_WARNING);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
        onlyOne = false;
    }

    void Window::systemWindow(Window &w)
    {
        w.BeginDrawing();
        w.ClearBackground(w.clearScreenColor);
        w.EndDrawing();
    }
} // namespace RType
