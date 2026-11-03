/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#ifndef SETUP_HPP_
#define SETUP_HPP_

#include "GameManager.hpp"
#include "ServerManager.hpp"

namespace RType {
    struct EnemyHooks;
}

namespace RType::Server {

    Entity createServer(GameManager &gm, short port);
    Entity createServer(GameManager &gm, ServerManager &&server);
    Entity createMasterClock(GameManager &gm);
    void createSystems(GameManager &gm, const EnemyHooks &enemyHooks);
} // namespace RType::Server

#endif
