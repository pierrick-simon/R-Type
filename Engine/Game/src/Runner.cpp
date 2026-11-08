/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#include <iostream>
#include <sol/sol.hpp>
#include <thread>

#include "ConfigBuilder.hpp"
#include "Console.hpp"
#include "Exceptions.hpp"
#include "Font.hpp"
#include "Game.hpp"
#include "GameManagerBinding.hpp"
#include "Model3D.hpp"
#include "Modules.hpp"
#include "Runner.hpp"
#include "Scripts.hpp"
#include "Texture2D.hpp"

namespace RType::Game {

    bool Runner::running = true;

    int Runner::run()
    {
        sol::state lua;

        lua.open_libraries(sol::lib::base, sol::lib::math);
        lua.set_exception_handler(&Scripts::handleException);
        GameManagerBinding::registerFunctions(lua);
#if defined(__DEBUG__)
        GameManager gm(true);
#else
        GameManager gm;
#endif

        try {
            ConfigBuilder::build(GameConfig, "config.conf");
            std::list<std::string> modules = Modules::resolve();

            Modules::check(modules);
            Modules::create(gm, modules);
            for (auto &path : Scripts::findPaths(GameConfig.srcDir))
                Scripts::load(lua, path, gm);
#if defined(__DEBUG__)
            Console console(lua, gm);

            std::thread(Console::listen).detach();
            gm.run(GameConfig.windowFPS, [&console] {
                console.run();
                return running;
            });
#else
            gm.run(GameConfig.windowFPS, [] { return running; });
#endif
        } catch (const EngineException &e) {
            std::cout << e.what() << std::endl;
            unloadGraphics(gm);
            return 84;
        }
        unloadGraphics(gm);
        return 0;
    }

    void Runner::unloadGraphics(GameManager &gm)
    {
        Texture2D::unloadTextures2D(gm);
        Model3D::unloadModels3D(gm);
        for (auto [e, _] : gm.view<Font>().getVector())
            gm.destroyEntity(e);
    }

} // namespace RType::Game
