/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineSystems
*/

#include "Exceptions.hpp"
#include "Game.hpp"

namespace RType {

    void Game::registerSystems(sol::state &lua)
    {
        lua["GameManager"]["addSystem"] =
            [](GameManager &gm, const std::string &name) {
                auto iter = systemFactories.find(name);
                if (iter == systemFactories.end())
                    throw UnknownSystem(name);
                return iter->second(gm);
            };
    }

}
