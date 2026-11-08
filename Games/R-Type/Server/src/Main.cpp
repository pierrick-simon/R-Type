/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>

#include "Connexion/MasterServer.hpp"
#include "GameManager.hpp"
#include "ServerManager.hpp"

int main()
{
    try {
        RType::GameManager gm;
        const RType::Entity master = RType::Server::createMasterServer(gm);
        auto &server = gm.getComponent<RType::ServerManager>(master);

        gm.run(60, [&] { return server.isRunning(); });
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
