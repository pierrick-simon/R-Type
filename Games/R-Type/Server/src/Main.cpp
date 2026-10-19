/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include "Background.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "Setup.hpp"
#include "Window.hpp"

int main()
{
    try {
        RType::GameManager gm;

        RType::Server::createWindow(gm);
        auto keyboardManager = RType::Server::createKeyboardManager(gm);
        RType::Server::createBackground(gm);
        RType::Server::createSpaceShip(gm, keyboardManager);
        RType::Server::createSystems(gm);

        while (!WindowShouldClose())
            gm.update();
    } catch (RType::EngineException &e) {
        std::cout << e.what() << std::endl;
        return 84;
    }
    return 0;
}
