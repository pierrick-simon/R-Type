/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#ifndef SETUP_HPP_
#define SETUP_HPP_

#include "GameManager.hpp"

namespace RType {
    struct EnemyHooks;
}

namespace RType::Server {

    Entity createWindow(GameManager &gm);
    Entity createKeyboardManager(GameManager &gm);
    void createSystems(GameManager &gm, const EnemyHooks &enemyHooks);
} // namespace RType::Server

#endif
