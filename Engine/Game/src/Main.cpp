/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include "Clock.hpp"
#include "Game.hpp"
#include "KeyboardManager.hpp"
#include "Window.hpp"

int main()
{
    try {
#if defined(__DEBUG__)
        RType::GameManager gm(true);
#else
        RType::GameManager gm;
#endif

        gm.addComponent<RType::Window>(gm.createEntity(),
            RType::Game.windowWidth,
            RType::Game.windowHeight,
            RType::Game.windowName);
        gm.addComponent<RType::KeyboardManager>(gm.createEntity());
        gm.addComponent<RType::MasterClock>(gm.createEntity());
        for (auto &init : RType::Game.inits)
            init(gm);
        gm.run(RType::Game.windowFPS, RType::Game.runningCondition);
    } catch (const RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
