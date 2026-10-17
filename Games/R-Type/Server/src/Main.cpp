/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include <unistd.h>
#include "GameManager.hpp"
#include "ServerManager.hpp"
#include "Clock.hpp"

RType::Entity createServer(RType::GameManager &gm)
{
    auto s = gm.createEntity();
    gm.addComponent<RType::ServerManager>(s, 12345);
    gm.createSystem<RType::ServerManager>([&gm](RType::ServerManager &server) {
        RType::ServerManager::systemServerReceived(server, gm);
    });
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
            if (elapsed.count() / 10000 > 1 && !yes) {
                yes = true;
                gm.getComponent<RType::ServerManager>(s).send();
            }
            usleep(1000);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
