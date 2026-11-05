/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include "Runner.hpp"
#include "Game.hpp"

RType::GameInfo RType::GameConfig = {};

int main()
{
    return RType::Game::Runner::run();
}
