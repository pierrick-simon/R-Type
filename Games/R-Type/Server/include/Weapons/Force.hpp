/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Force
*/

#ifndef FORCE_HPP_
#define FORCE_HPP_

#include <optional>

#include "Components.hpp"
#include "GameManager.hpp"

namespace RType {
    struct SpriteSheet2D;
} // namespace RType

namespace RType::Server {

    std::optional<Entity> findForce(GameManager &gm, Entity player);
    void giveForce(GameManager &gm, Entity player, LaserType laser);
    void forceSystem(GameManager &gm);
    void forceAnimation(const Force &force, SpriteSheet2D &sheet);
} // namespace RType::Server

#endif
