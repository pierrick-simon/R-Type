/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Game
*/

#include <algorithm>
#include <filesystem>

#include "Exceptions.hpp"
#include "Game.hpp"

namespace RType {

    int Game::run()
    {
        sol::state lua;

        lua.open_libraries(sol::lib::base);
        registerGameManager(lua);
        try {
#if defined(__DEBUG__)
            GameManager gm(true);
#else
            GameManager gm;
#endif
            std::list<std::string> modules = getModules();

            checkModules(modules);
            createModules(gm, modules);
            for (auto &path: getScriptPaths("TestLua"))
                loadScriptPath(lua, path, gm);
            gm.run(GameConfig.windowFPS, GameConfig.runningCondition);
        } catch (const EngineException &e) {
            std::cout << e.what() << std::endl;
            return 84;
        }
        return 0;
    }

    std::list<std::string> Game::getModules()
    {
        std::list<std::string> modules;
        auto addModule = [&modules](const std::string &module) {
            if (std::ranges::find(modules, module) == modules.end())
                modules.push_back(module);
        };

        std::ranges::for_each(GameConfig.modules, addModule);
        std::ranges::for_each(defaultModules, addModule);
        return modules;
    }

    void Game::checkModules(const std::list<std::string> &modules)
    {
        for (auto &module : modules) {
            auto iter = moduleAllowed.find(module);
            if (iter == moduleAllowed.end())
                throw NoSuchModule(module);
            for (auto &dependency : iter->second.dependencies)
                if (std::ranges::find(modules, dependency) == modules.end())
                    throw MissingModule(module, dependency);
        }
    }

    void Game::createModules(
        GameManager &gm, const std::list<std::string> &modules)
    {
        for (auto &module : modules)
            moduleAllowed.at(module).create(gm, GameConfig);
    }

    std::list<std::string> Game::getScriptPaths(const std::string &directory)
    {
        std::list<std::string> paths;

        if (!std::filesystem::is_directory(directory))
            return paths;
        for (auto &entry : std::filesystem::recursive_directory_iterator(directory,
                std::filesystem::directory_options::skip_permission_denied))
            if (entry.is_regular_file() && entry.path().extension() == ".lua")
                paths.push_back(entry.path().string());
        paths.sort();
        return paths;
    }

    void Game::loadScriptPath(sol::state &lua, const std::string &path, GameManager &gm)
    {
        sol::environment env(lua, sol::create, lua.globals());

        checkScriptResult(
            lua.script_file(path, env, &sol::script_pass_on_error));
        checkScriptResult(env["init"](gm));
    }

    void Game::checkScriptResult(const sol::protected_function_result &result)
    {
        if (!result.valid()) {
            sol::error error = result;
            throw ScriptError(error.what());
        }
    }

}
