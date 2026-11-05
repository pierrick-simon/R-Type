/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManagerBinding
*/

#include <string>

#include "ComponentBinding.hpp"
#include "GameManager.hpp"
#include "GameManagerBinding.hpp"
#include "InputBinding.hpp"
#include "Runner.hpp"
#include "SystemBinding.hpp"
#include "TimeBinding.hpp"
#include "ViewBinding.hpp"
#include "WindowBinding.hpp"

namespace RType::Game {

    void GameManagerBinding::registerFunctions(sol::state &lua)
    {
        lua.new_usertype<GameManager>("GameManager", sol::no_constructor);
        lua.new_usertype<Entity>("Entity",
            sol::no_constructor,
            sol::meta_function::to_string,
            [](const Entity &entity) {
                return "#" + std::to_string(entity.index()) + " (" +
                    std::to_string(entity.generation()) + ")";
            });
        lua["GameManager"]["createEntity"] = &GameManager::createEntity;
        lua["GameManager"]["destroyEntity"] = &GameManager::destroyEntity;
        lua["GameManager"]["hasEntity"] = &GameManager::hasEntity;
        lua["GameManager"]["stop"] = [](GameManager &) {
            Runner::running = false;
        };
        ComponentBinding::registerFunctions(lua);
        SystemBinding::registerFunctions(lua);
        ViewBinding::registerFunctions(lua);
        InputBinding::registerFunctions(lua);
        TimeBinding::registerFunctions(lua);
        WindowBinding::registerFunctions(lua);
    }

} // namespace RType::Game
