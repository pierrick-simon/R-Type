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
#include <sol/sol.hpp>

#include "Clock.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"
#include "Window.hpp"

namespace RType {

    struct GameInfo {
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

    /**
     * @brief Entry point of the engine: builds the game described by
     * GameConfig and runs it.
     */
    class Game {
    public:
        Game() = delete;

        /**
         * @brief Loads the modules, calls the game's inits, then runs the
         * main loop until GameConfig.runningCondition returns false.
         * @return 0 on success, 84 if the engine raised an error.
         */
        static int run();

    private:
        /**
         * @brief Merges the modules asked by the game with the default
         * ones, without duplicates, keeping the game's order first.
         */
        static std::list<std::string> getModules();

        /**
         * @brief Checks that every module exists and that its dependencies
         * are loaded too.
         * @throw NoSuchModule If a module is not in moduleAllowed.
         * @throw MissingModule If a dependency of a module is missing.
         */
        static void checkModules(const std::list<std::string> &modules);

        /**
         * @brief Creates every module, in order.
         */
        static void createModules(
            GameManager &gm, const std::list<std::string> &modules);
        
        /**
         * @brief Lists every Lua script inside a directory, including its
         * subdirectories.
         * @param directory The directory to search in.
         * @return The path of every .lua file, sorted alphabetically. Empty
         * if the directory does not exist.
         */
        static std::list<std::string> getScriptPaths(
            const std::string &directory);

        /**
         * @brief Runs a Lua script and adds the functions it defines to
         * GameConfig.inits, so that they are called like the game's own.
         * @param path The path of the .lua file to load.
         */
        static void loadScriptPath(sol::state &lua, const std::string &path, GameManager &gm);
    };

} // namespace RType

#endif /* !GAME_HPP_ */
