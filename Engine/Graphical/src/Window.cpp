/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Window
*/

#include "Window.hpp"
#include "Exceptions.hpp"

namespace RType {
    Window::Window(int width, int height, const std::string &name,
        raylib::Color clearScreenColor) :
        clearScreenColor(clearScreenColor)
    {
        static bool onlyOne = true;
        if (!onlyOne)
            throw EngineException("Window can be created only once");
        try {
            _window = std::make_unique<raylib::Window>();
            _window->Init(width, height, name, LOG_WARNING);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
        onlyOne = false;
    }

    void Window::systemClearWindow(const Window &w)
    {
        static int fps = w._window->GetFPS();

        if (fps != w.fps) {
            fps = w.fps;
            w._window->SetTargetFPS(w.fps);
        }
        w._window->BeginDrawing();
        w._window->ClearBackground(w.clearScreenColor);
        w._window->EndDrawing();
    }
} // namespace RType
