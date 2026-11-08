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
            SetTraceLogLevel(LOG_NONE);
            _window = std::make_unique<raylib::Window>();
            _window->Init(width, height, name, LOG_WARNING);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
        onlyOne = false;
    }

    void Window::systemClearWindow(Window &w)
    {
        w.setTargetFps();
        w.beginDrawing();
        w.endDrawing();
    }

    void Window::beginDrawing()
    {
        if (_window->IsFullscreen() != fullScreen)
            _window->ToggleFullscreen();
        if (IsCursorHidden() != this->hideCursor) {
            if (this->hideCursor)
                DisableCursor();
            else
                EnableCursor();
        }
        this->_window->BeginDrawing();
        this->_window->ClearBackground(this->clearScreenColor);
    }

    void Window::endDrawing()
    {
        this->_window->EndDrawing();
    }

    void Window::setTargetFps()
    {
        static int fps = this->_window->GetFPS();

        if (fps != this->fps) {
            fps = this->fps;
            this->_window->SetTargetFPS(this->fps);
        }
    }

    bool Window::shouldClose()
    {
        return _window->ShouldClose();
    }
} // namespace RType
