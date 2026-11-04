/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>

#include "GameManager.hpp"
#include "MasterServer.hpp"
#include "ServerManager.hpp"

int main()
{
    try {
        RType::GameManager gm;
        const RType::Entity master = RType::Server::createMasterServer(gm);
        auto &server = gm.getComponent<RType::ServerManager>(master);

        gm.run(60, [&] { return server.isRunning(); });
    } catch (const RType::EngineException &e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
