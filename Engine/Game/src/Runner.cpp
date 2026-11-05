/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#include <iostream>
#include <sol/sol.hpp>

#include "Exceptions.hpp"
#include "Game.hpp"
#include "GameManagerBinding.hpp"
#include "Modules.hpp"
#include "Runner.hpp"
#include "Scripts.hpp"

namespace RType::Game {

    int Runner::run()
    {
        sol::state lua;

        lua.open_libraries(sol::lib::base);
        lua.set_exception_handler(&Scripts::handleException);
        GameManagerBinding::registerFunctions(lua);
        try {
#if defined(__DEBUG__)
            GameManager gm(true);
#else
            GameManager gm;
#endif
            std::list<std::string> modules = Modules::resolve();

            Modules::check(modules);
            Modules::create(gm, modules);
            for (auto &path : Scripts::findPaths("TestLua"))
                Scripts::load(lua, path, gm);
            gm.run(GameConfig.windowFPS, GameConfig.runningCondition);
        } catch (const EngineException &e) {
            std::cout << e.what() << std::endl;
            return 84;
        }
        return 0;
    }

} // namespace RType::Game
