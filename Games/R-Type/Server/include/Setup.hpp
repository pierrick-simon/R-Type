/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#ifndef SETUP_HPP_
#define SETUP_HPP_

#include "GameManager.hpp"
#include "Texture.hpp"
#include "ServerManager.hpp"

namespace RType {
    struct EnemyHooks;
}

namespace RType::Server {

    struct TextureCleanup {
        RType::GameManager &gm;

        ~TextureCleanup()
        {
            try {
                RType::Server::unloadTextures(gm);
            } catch (...) {
            }
        }
    };

    Entity createServer(GameManager &gm, short port);
    Entity createServer(GameManager &gm, ServerManager &&server);
    Entity createMasterClock(GameManager &gm);
    void createSystems(GameManager &gm, const EnemyHooks &enemyHooks);
} // namespace RType::Server

#endif
