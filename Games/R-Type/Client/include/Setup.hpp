/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#ifndef SETUP_HPP_
#define SETUP_HPP_

#include "GameManager.hpp"

namespace RType::Client {
    Entity createWindow(GameManager &gm);
    void createClientSystems(GameManager &gm);
    Entity createControl(GameManager &gm);
    Entity createMasterClock(GameManager &gm);
    void createSystems(GameManager &gm);
} // namespace RType::Client

#endif /* !SETUP_HPP_ */
