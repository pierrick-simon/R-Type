/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WindowBinding
*/

#include "GameManagerBinding.hpp"
#include "Window.hpp"
#include "WindowBinding.hpp"

namespace RType::Game {

    void WindowBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["windowShouldClose"] = [](GameManager &gm) {
            GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:windowShouldClose", "Window");
            return WindowShouldClose();
        };
    }

} // namespace RType::Game
