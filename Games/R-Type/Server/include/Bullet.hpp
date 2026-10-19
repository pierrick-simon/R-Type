/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Bullet
*/

#ifndef BULLET_HPP_
#define BULLET_HPP_

#include "Components.hpp"
#include "GameManager.hpp"
#include "Position2D.hpp"

namespace RType::Server {

    void spawnBullet(GameManager &gm, const Position2D &shooter);
    void bulletSystem(GameManager &gm);

}

#endif
