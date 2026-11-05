/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PowerUp
*/

#ifndef POWERUP_HPP_
#define POWERUP_HPP_

#include "Components.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    Entity spawnPowerUp(GameManager &gm, LaserType laser, float x, float y);
    void dropPowerUp(GameManager &gm, float x, float y);
    void powerUpPickupSystem(GameManager &gm);
} // namespace RType::Server

#endif
