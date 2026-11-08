/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WindowBinding
*/

#include "WindowBinding.hpp"
#include "GameManagerBinding.hpp"
#include "Window.hpp"

namespace RType::Game {

    void WindowBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["shouldClose"] = [](GameManager &gm) {
            GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:shouldClose", "Window")
                .shouldClose();
        };
    }

} // namespace RType::Game
