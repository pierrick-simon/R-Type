/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Window
*/

#include "Window.hpp"

namespace RType {
    void Window::systemWindow(Window &w)
    {
        w.window.BeginDrawing();
        ClearBackground(w.clearScreenColor);
        w.window.EndDrawing();
    }
} // namespace RType
