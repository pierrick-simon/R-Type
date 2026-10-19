/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Player
*/

#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "Clock.hpp"
#include "Components.hpp"
#include "GameManager.hpp"
#include "Keyboard.hpp"
#include "Position2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    Entity createSpaceShip(GameManager &gm, Entity keyboardManager);
    void playerHandle(GameManager &gm, Player &player, Velocity2D &velocity,
        const Position2D &pos, const Keyboard &keyboard, const Clock &clock);
    void playerClamp(
        const Player &player, Position2D &pos, Velocity2D &velocity);
    void playerAnimation(
        const Player &player, const Velocity2D &velocity, SpriteSheet2D &sheet);

} // namespace RType::Server

#endif
