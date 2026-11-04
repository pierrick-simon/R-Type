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
         * @brief Runs a Lua script in its own environment, then calls its
         * init function with the GameManager.
         * @param path The path of the .lua file to load.
         */
        static void loadScriptPath(sol::state &lua, const std::string &path, GameManager &gm);

        /**
         * @brief Turns a failed Lua call into an engine error, so that it
         * stops the game instead of being silently ignored.
         * @throw ScriptError If the call raised a Lua error.
         */
        static void checkScriptResult(
            const sol::protected_function_result &result);

        /**
         * @brief Lets scripts add the engine's own systems by name, with
         * gm:addSystem("DrawSprite2D"). The name is the C++ function's
         * without its "system" prefix.
         * @throw UnknownSystem If the name is not an engine system.
         */
        static void registerSystems(sol::state &lua);

        /**
         * @brief Creates one engine system, as registered by
         * registerSystems.
         */
        using SystemFactory = std::function<SystemId(GameManager &)>;

        /**
         * @brief Wraps an engine system into a factory that creates it, with
         * the components read from the system's own parameters. Defined in
         * EngineSystems.cpp, the only place that uses it.
         */
        template <typename... Components>
        static SystemFactory makeSystem(void (*system)(Components...));

        /**
         * @brief Same, for a system that also takes the GameManager first.
         */
        template <typename... Components>
        static SystemFactory makeSystem(
            void (*system)(GameManager &, Components...));

        /**
         * @brief Every engine system scripts can add, by name. Defined in
         * EngineSystems.cpp.
         */
        static const std::unordered_map<std::string, SystemFactory>
            systemFactories;

        /**
         * @brief Lets scripts create entities and give them the engine's
         * components by name, with gm:createEntity() and
         * gm:addComponent(entity, "Position2D", {x = 10, y = 20}). Fields
         * left out keep the component's default value.
         * @throw UnknownComponent If the name is not an engine component.
         */
        static void registerComponents(sol::state &lua);

        /**
         * @brief Adds one engine component to an entity, built from the
         * table given by the script (or its defaults if there is none).
         */
        using ComponentFactory = std::function<void(
            GameManager &, Entity, const sol::optional<sol::table> &)>;

        /**
         * @brief Every component scripts can add, by name. Defined in
         * EngineComponents.cpp.
         */
        static const std::unordered_map<std::string, ComponentFactory>
            componentFactories;

        static void registerGameManager(sol::state &lua)
        {
            lua.new_usertype<GameManager>("GameManager", sol::no_constructor);
            registerSystem<>(lua);
            lua["GameManager"]["createEntity"] = &GameManager::createEntity;
            lua["GameManager"]["destroyEntity"] = &GameManager::destroyEntity;
            lua["GameManager"]["hasEntity"] = &GameManager::hasEntity;
            registerSystems(lua);
            registerComponents(lua);
        }

        /**
         * @brief Exposes GameManager to Lua, so that scripts can call
         * gm:createSystem(func). The given Lua function is called each
         * update() with the GameManager.
         */
        template <typename... Components>
        static void registerSystem(sol::state &lua)
        {
            
                lua["GameManager"]["createSystem"] = [](GameManager &gm, sol::protected_function f) {
                    return gm.createSystem<Components...>(
                        [f](GameManager &gm, Components &...c) {
                            checkScriptResult(f(gm, c...));
                        });
                };
        }

    };

} // namespace RType

#endif /* !GAME_HPP_ */
