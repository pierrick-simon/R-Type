/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include <unistd.h>
#include "Clock.hpp"
#include "GameManager.hpp"
#include "ServerManager.hpp"

RType::Entity createServer(RType::GameManager &gm)
{
    auto s = gm.createEntity();
    gm.addComponent<RType::ServerManager>(s, 12345);
    gm.createSystem<RType::ServerManager>(
        RType::ServerManager::systemServerReceived);
    return s;
}

int main()
{
    try {
        RType::GameManager gm;
        auto s = createServer(gm);
        auto c = gm.createEntity();
        gm.addComponent<RType::Clock>(c);

        gm.createSystem<RType::Clock>(RType::Clock::systemClock);

        auto elapsed = std::chrono::milliseconds(0);
        bool yes = false;
        while (true) {
            gm.update();
            elapsed += gm.getComponent<RType::Clock>(c).getElapsed();
            if (elapsed.count() / 5000 > 1 && !yes) {
                yes = true;
                std::cout << elapsed << "\n";
            }
            usleep(1000);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
