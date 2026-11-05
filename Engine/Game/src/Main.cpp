/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include "Game.hpp"
#include "Runner.hpp"

RType::GameInfo RType::GameConfig = {};

int main()
{
    return RType::Game::Runner::run();
}
