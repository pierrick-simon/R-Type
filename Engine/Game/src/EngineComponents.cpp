/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineComponents
*/

#include "Exceptions.hpp"
#include "Game.hpp"

namespace RType {

    std::unordered_map<std::string, Game::ComponentEntry>
        Game::componentEntries;

    const char *const Game::setFieldsScript = R"(
        local component, fields = ...
        for key, value in pairs(fields) do
            local found, current = pcall(function() return component[key] end)
            if not found or current == nil then
                return "no field '" .. tostring(key) .. "'"
            end
            if type(value) ~= type(current) then
                return "field '" .. tostring(key) .. "' expects a "
                    .. type(current) .. ", got a " .. type(value)
            end
            local ok, err = pcall(function() component[key] = value end)
            if not ok then
                return "field '" .. tostring(key) .. "': " .. tostring(err)
            end
        end
        return nil
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
            getEntry(name).add(gm, entity, args);
        };
        lua["GameManager"]["removeComponent"] =
            [](GameManager &gm, Entity entity, const std::string &name) {
                getEntry(name).remove(gm, entity);
            };
        lua["GameManager"]["hasComponent"] =
            [](GameManager &gm, Entity entity, const std::string &name) {
                return getEntry(name).has(gm, entity);
            };
        lua["GameManager"]["getComponent"] = [](GameManager &gm,
                                                Entity entity,
                                                const std::string &name,
                                                sol::this_state L) {
            return getEntry(name).get(L, gm, entity);
        };
        lua["GameManager"]["countComponents"] =
            [](GameManager &gm, const std::string &name) {
                return getEntry(name).count(gm);
            };

        bindEngineComponents(lua);
    }

    const Game::ComponentEntry &Game::getEntry(const std::string &name)
    {
        auto iter = componentEntries.find(name);

        if (iter == componentEntries.end())
            throw UnknownComponent(name);
        return iter->second;
    }

}
