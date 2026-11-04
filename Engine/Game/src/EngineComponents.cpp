/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineComponents
*/

#include "Exceptions.hpp"
#include "Game.hpp"

namespace RType {

    std::unordered_map<std::string, Game::ComponentFactory>
        Game::componentFactories;

    const char *const Game::setFieldsScript = R"(
        local component, fields = ...
        for key, value in pairs(fields) do
            local ok, err = pcall(function() component[key] = value end)
            if not ok then
                error("field '" .. tostring(key) .. "': " .. tostring(err), 0)
            end
        end
    )";

    void Game::registerComponents(sol::state &lua)
    {
        lua.registry()["setFields"] =
            lua.load(setFieldsScript, "=setFields")
                .get<sol::protected_function>();
        lua["GameManager"]["addComponent"] = [](GameManager &gm,
                                                Entity entity,
                                                const std::string &name,
                                                const ComponentArgs &args) {
            auto iter = componentFactories.find(name);
            if (iter == componentFactories.end())
                throw UnknownComponent(name);
            iter->second(gm, entity, args);
        };

        bindEngineComponents(lua);
    }

}
