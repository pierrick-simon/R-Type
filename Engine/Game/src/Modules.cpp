/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Modules
*/

#include <algorithm>

#include "Clock.hpp"
#include "Exceptions.hpp"
#include "KeyboardManager.hpp"
#include "Listener.hpp"
#include "Modules.hpp"
#include "MouseManager.hpp"
#include "Sound.hpp"
#include "Window.hpp"

namespace RType::Game {

    const std::unordered_map<std::string, Module> Modules::allowed = {
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
        {"Audio",
            {{},
                [](GameManager &gm, const GameInfo &) {
                    gm.addComponent<Listener>(gm.createEntity());
                    gm.createSystem<Sound>(Sound::systemSound);
                }}},
        {"Clock",
            {{},
                [](GameManager &gm, const GameInfo &) {
                    gm.addComponent<MasterClock>(gm.createEntity());
                    gm.createSystem<MasterClock>(
                        MasterClock::systemMasterClock);
                }}},
    };

    const std::list<std::string> Modules::defaults = {"Clock"};

    std::list<std::string> Modules::resolve()
    {
        std::list<std::string> modules;
        auto addModule = [&modules](const std::string &module) {
            if (std::ranges::find(modules, module) == modules.end())
                modules.push_back(module);
        };

        std::ranges::for_each(GameConfig.modules, addModule);
        std::ranges::for_each(defaults, addModule);
        return modules;
    }

    void Modules::check(const std::list<std::string> &modules)
    {
        for (auto &module : modules) {
            auto iter = allowed.find(module);
            if (iter == allowed.end())
                throw NoSuchModule(module);
            for (auto &dependency : iter->second.dependencies)
                if (std::ranges::find(modules, dependency) == modules.end())
                    throw MissingModule(module, dependency);
        }
    }

    void Modules::create(GameManager &gm, const std::list<std::string> &modules)
    {
        for (auto &module : modules)
            allowed.at(module).create(gm, GameConfig);
    }

} // namespace RType::Game
