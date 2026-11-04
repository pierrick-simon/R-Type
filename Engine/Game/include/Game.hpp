/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Game
*/

#ifndef GAME_HPP_
#define GAME_HPP_

#include <chrono>
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <sol/sol.hpp>

#include "Clock.hpp"
#include "Exceptions.hpp"
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
         * @brief Turns a C++ exception thrown while Lua was calling the
         * engine into a Lua error. An EngineException is passed without its
         * "Engine Error: " prefix, which checkScriptResult adds back once.
         */
        static int handleException(lua_State *L,
            sol::optional<const std::exception &> exception,
            sol::string_view description);

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
         * the components read from the system's own parameters.
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
         * EngineBindings.cpp, the only file to edit to make a new system
         * usable from Lua.
         */
        static const std::unordered_map<std::string, SystemFactory>
            systemFactories;

        /**
         * @brief Lets scripts give the engine's components to entities by
         * name, with gm:addComponent(entity, "Position2D", {x = 10}).
         * Fields left out keep the component's default value.
         * @throw UnknownComponent If the name is not an engine component.
         */
        static void registerComponents(sol::state &lua);

        /**
         * @brief Every engine component scripts can use, one bindComponent
         * each. Defined in EngineBindings.cpp, the only file to edit to
         * make a new component usable from Lua.
         */
        static void bindEngineComponents(sol::state &lua);

        /**
         * @brief Makes a component usable from Lua under the given name:
         * scripts can then add it with gm:addComponent and read or write
         * the given fields, which are passed as sol::usertype expects them
         * ("x", &Position2D::x, ...).
         *
         * A component without a default constructor is built from the
         * "path" field of the script's table instead.
         */
        template <typename Component, typename... Fields>
        static void bindComponent(
            sol::state &lua, const std::string &name, Fields &&...fields);

        /**
         * @brief Adds one component to an entity, built from the table
         * given by the script (or its defaults if there is none).
         */
        using ComponentArgs = sol::optional<sol::table>;
        using ComponentFactory =
            std::function<void(GameManager &, Entity, const ComponentArgs &)>;

        /**
         * @brief Every component scripts can add, by name, filled by
         * bindComponent.
         */
        static std::unordered_map<std::string, ComponentFactory>
            componentFactories;

        /**
         * @brief Lua code that copies every field of the script's table
         * onto the component, through its usertype. Written in Lua so that
         * a wrong field stops inside Lua and comes back as a normal error,
         * instead of jumping over C++ code.
         */
        static const char *const setFieldsScript;

        /**
         * @brief Builds a component from the script's table: its defaults
         * overwritten by the table's fields, or, for a component that has
         * no default constructor, from the table's "path".
         * @throw InvalidComponentArgs If a field does not exist or has the
         * wrong type, or if the path is missing.
         */
        template <typename Component>
        static Component makeComponent(
            const std::string &name, const ComponentArgs &args);

        /**
         * @brief Exposes a std::chrono::milliseconds member to Lua as a
         * plain number of milliseconds, for bindComponent.
         */
        template <typename Component>
        static auto milliseconds(std::chrono::milliseconds Component::*member);

        /**
         * @brief Exposes a member of a member to Lua as if it was a direct
         * field, such as SpriteSheet2D::recNb.height as "rows", for
         * bindComponent.
         */
        template <typename Component, typename Inner, typename T>
        static auto nested(Inner Component::*outer, T Inner::*inner);

        /**
         * @brief Exposes GameManager and Entity to Lua, with everything
         * scripts can do on them: entities, systems and components. Called
         * once, before any script is loaded.
         */
        static void registerGameManager(sol::state &lua);

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

    template <typename... Components>
    Game::SystemFactory Game::makeSystem(void (*system)(Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

    template <typename... Components>
    Game::SystemFactory Game::makeSystem(
        void (*system)(GameManager &, Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

    template <typename Component>
    auto Game::milliseconds(std::chrono::milliseconds Component::*member)
    {
        return sol::property(
            [member](Component &c) {
                return (c.*member).count();
            },
            [member](Component &c, std::int64_t ms) {
                c.*member = std::chrono::milliseconds(ms);
            });
    }

    template <typename Component, typename Inner, typename T>
    auto Game::nested(Inner Component::*outer, T Inner::*inner)
    {
        return sol::property(
            [outer, inner](Component &c) {
                return c.*outer.*inner;
            },
            [outer, inner](Component &c, T value) {
                c.*outer.*inner = value;
            });
    }

    template <typename Component>
    Component Game::makeComponent(
        const std::string &name, const ComponentArgs &args)
    {
        if constexpr (std::is_default_constructible_v<Component>) {
            Component component {};

            if (args) {
                sol::state_view lua(args->lua_state());
                sol::protected_function setFields =
                    lua.registry()["setFields"];
                sol::protected_function_result result =
                    setFields(&component, *args);

                if (!result.valid()) {
                    sol::error error = result;
                    throw InvalidComponentArgs(name, error.what());
                }
            }
            return component;
        } else {
            sol::optional<std::string> path;

            if (args)
                path = args->get<sol::optional<std::string>>("path");
            if (!path)
                throw InvalidComponentArgs(name, "missing 'path'");
            return Component(*path);
        }
    }

    template <typename Component, typename... Fields>
    void Game::bindComponent(
        sol::state &lua, const std::string &name, Fields &&...fields)
    {
        lua.new_usertype<Component>(name, std::forward<Fields>(fields)...);
        componentFactories[name] = [name](GameManager &gm, Entity entity,
                                       const ComponentArgs &args) {
            gm.addComponent<Component>(
                entity, makeComponent<Component>(name, args));
        };
    }

} // namespace RType

#endif /* !GAME_HPP_ */
