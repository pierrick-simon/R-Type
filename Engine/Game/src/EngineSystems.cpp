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

    void Game::registerSystem(sol::state &lua)
    {
        lua["GameManager"]["createSystem"] = sol::overload(
            [](GameManager &gm, const sol::protected_function &func) {
                return gm.createSystem<>([func](GameManager &gm) {
                    checkScriptResult(func(gm));
                });
            },
            [](GameManager &gm, const sol::table &names,
                const sol::protected_function &func) {
                std::vector<const ComponentEntry *> entries =
                    resolveComponents(names);

                return gm.createSystem<>([entries, func](GameManager &gm) {
                    if (entries.empty())
                        checkScriptResult(func(gm));
                    else
                        runScriptSystem(gm, entries, func);
                });
            });
    }

    void Game::runScriptSystem(GameManager &gm,
        const std::vector<const ComponentEntry *> &entries,
        const sol::protected_function &func)
    {
        const ComponentEntry *smallest = nullptr;
        std::size_t smallestCount = 0;

        for (const ComponentEntry *entry : entries) {
            std::size_t count = entry->count(gm);

            if (count == 0)
                return;
            if (!smallest || count < smallestCount) {
                smallest = entry;
                smallestCount = count;
            }
        }

        lua_State *L = func.lua_state();
        std::vector<sol::object> components(entries.size());

        smallest->forEachEntity(gm, [&](Entity entity) {
            for (std::size_t i = 0; i < entries.size(); i++) {
                if (!entries[i]->has(gm, entity))
                    return;
                components[i] = entries[i]->get(L, gm, entity);
            }
            checkScriptResult(func(gm, entity, sol::as_args(components)));
        });
    }

}
