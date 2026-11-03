/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <algorithm>

#include "Exceptions.hpp"
#include "Game.hpp"

int main()
{
    try {
#if defined(__DEBUG__)
        RType::GameManager gm(true);
#else
        RType::GameManager gm;
#endif

        std::list<std::string> modules;
        auto addModule = [&modules](const std::string &module) {
            if (std::ranges::find(modules, module) == modules.end())
                modules.push_back(module);
        };

        std::ranges::for_each(RType::GameConfig.modules, addModule);
        std::ranges::for_each(RType::defaultModules, addModule);
        for (auto &module : modules) {
            auto iter = RType::moduleAllowed.find(module);
            if (iter == RType::moduleAllowed.end())
                throw RType::NoSuchModule(module);
            iter->second(gm, RType::GameConfig);
        }
        for (auto &init : RType::GameConfig.inits)
            init(gm);
        gm.run(RType::GameConfig.windowFPS, RType::GameConfig.runningCondition);
    } catch (const RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
