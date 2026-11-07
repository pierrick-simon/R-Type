/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#include <functional>
#include <iostream>
#include <sol/sol.hpp>
#include <thread>

#include "ConfigBuilder.hpp"
#include "Console.hpp"
#include "Exceptions.hpp"
#include "Game.hpp"
#include "GameManagerBinding.hpp"
#include "Modules.hpp"
#include "Runner.hpp"
#include "Scripts.hpp"

namespace RType::Game {

    std::atomic<bool> Runner::running = true;

    int Runner::run()
    {
        sol::state lua;

#if defined(__DEBUG__)
        std::ios::sync_with_stdio(false);
#endif
        lua.open_libraries(sol::lib::base, sol::lib::math);
        lua.set_exception_handler(&Scripts::handleException);
        GameManagerBinding::registerFunctions(lua);
        try {
            ConfigBuilder::build(GameConfig, "config.conf");
#if defined(__DEBUG__)
            GameManager gm(true);
#else
            GameManager gm;
#endif
            std::list<std::string> modules = Modules::resolve();

            Modules::check(modules);
            Modules::create(gm, modules);
            for (auto &path : Scripts::findPaths(GameConfig.srcDir))
                Scripts::load(lua, path, gm);
#if defined(__DEBUG__)
            std::thread(Console::listen, std::cref(running)).detach();
            gm.run(GameConfig.windowFPS, [&lua, &gm] {
                Console::run(lua, gm);
                return running.load();
            });
#else
            gm.run(GameConfig.windowFPS, [] { return running.load(); });
#endif
        } catch (const EngineException &e) {
            std::cout << e.what() << std::endl;
            return 84;
        }
        return 0;
    }

} // namespace RType::Game
