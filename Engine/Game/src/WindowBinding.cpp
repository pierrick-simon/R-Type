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
            return GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:shouldClose", "Window")
                .shouldClose();
        };
        lua["GameManager"]["fps"] = [](GameManager &gm) {
            return GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:fps", "Window")
                .fps;
        };
        lua["GameManager"]["setFps"] = [](GameManager &gm, int fps) {
            GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:setFps", "Window")
                .fps = fps;
        };
        lua["GameManager"]["fullScreen"] = [](GameManager &gm) {
            return GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:fullScreen", "Window")
                .fullScreen;
        };
        lua["GameManager"]["setFullScreen"] = [](GameManager &gm,
                                                  bool fullScreen) {
            GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:setFullScreen", "Window")
                .fullScreen = fullScreen;
        };
        lua["GameManager"]["hideCursor"] = [](GameManager &gm) {
            return GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:hideCursor", "Window")
                .hideCursor;
        };
        lua["GameManager"]["setHideCursor"] = [](GameManager &gm,
                                                  bool hideCursor) {
            GameManagerBinding::findModuleComponent<Window>(
                gm, "gm:setHideCursor", "Window")
                .hideCursor = hideCursor;
        };
    }

} // namespace RType::Game
