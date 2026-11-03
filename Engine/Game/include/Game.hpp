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
#include <unordered_map>

#include "Clock.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"
#include "Window.hpp"

namespace RType {

    struct GameInfo {
        std::list<std::function<void(RType::GameManager &)>> inits = {};
        std::function<bool()> runningCondition;
        int windowFPS = 60;
        std::size_t windowWidth = 800;
        std::size_t windowHeight = 600;
        std::string windowName = "My Game";
        std::list<std::string> modules;
    };

    extern GameInfo GameConfig;

    /**
     * @brief An engine feature the game can ask for in GameInfo::modules.
     */
    struct Module {
        std::list<std::string> dependencies;
        std::function<void(GameManager &, const GameInfo &)> create;
    };

    static const std::unordered_map<std::string, Module> moduleAllowed = {
        {"Window",
            {{},
                [](GameManager &gm, const GameInfo &info) {
                    gm.addComponent<Window>(gm.createEntity(),
                        info.windowWidth,
                        info.windowHeight,
                        info.windowName);
                    gm.createSystem<Window>(Window::systemClearWindow);
                }}},
        {"Keyboard",
            {{"Window"},
                [](GameManager &gm, const GameInfo &) {
                    gm.addComponent<KeyboardManager>(gm.createEntity());
                    gm.createSystem<KeyboardManager>(
                        KeyboardManager::systemKeyboardManager);
                }}},
        {"Mouse",
            {{"Window"},
                [](GameManager &gm, const GameInfo &) {
                    gm.addComponent<MouseManager>(gm.createEntity());
                    gm.createSystem<MouseManager>(
                        MouseManager::systemMouseManager);
                }}},
        {"Clock",
            {{},
                [](GameManager &gm, const GameInfo &) {
                    gm.addComponent<MasterClock>(gm.createEntity());
                    gm.createSystem<MasterClock>(
                        MasterClock::systemMasterClock);
                }}},
    };

    static const std::list<std::string> defaultModules = {"Clock"};

} // namespace RType

#endif /* !GAME_HPP_ */
