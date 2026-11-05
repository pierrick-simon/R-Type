/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineSystems
*/

#include <algorithm>
#include <memory>

#include "Exceptions.hpp"
#include "Game.hpp"

namespace RType {

    void Game::checkNotUpdating(const GameManager &gm, const char *function)
    {
        if (gm.isUpdating())
            throw SystemCreatedInSystem(function);
    }

    void Game::registerSystems(sol::state &lua)
    {
        lua["GameManager"]["addSystem"] =
            [](GameManager &gm, const std::string &name) {
                checkNotUpdating(gm, "gm:addSystem");
                auto iter = systemFactories.find(name);
                if (iter == systemFactories.end())
                    throw UnknownSystem(name);
                return iter->second(gm);
            };
        lua["GameManager"]["destroySystem"] =
            [] (GameManager &gm, const SystemId id) {
                gm.destroySystem(id);
            };
        lua["GameManager"]["hasSystem"] =
            [] (GameManager &gm, const SystemId id) {
                return gm.hasSystem(id);
            };
    }

    void Game::registerSystem(sol::state &lua)
    {
        lua["GameManager"]["createSystem"] = sol::overload(
            [](GameManager &gm, const sol::protected_function &func) {
                checkNotUpdating(gm, "gm:createSystem");
                return gm.createSystem<>([func](GameManager &gm) {
                    checkScriptResult(func(gm));
                });
            },
            [](GameManager &gm, const sol::table &names,
                const sol::protected_function &func) {
                checkNotUpdating(gm, "gm:createSystem");
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
        lua_State *L = func.lua_state();
        std::vector<sol::object> components(entries.size());

        forEachMatch(gm, entries, [&](Entity entity) {
            for (std::size_t i = 0; i < entries.size(); i++)
                components[i] = entries[i]->get(L, gm, entity);
            checkScriptResult(func(gm, entity, sol::as_args(components)));
        });
    }

    void Game::forEachMatch(GameManager &gm,
        const std::vector<const ComponentEntry *> &entries,
        const std::function<void(Entity)> &func)
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
        if (!smallest)
            return;
        smallest->forEachEntity(gm, [&](Entity entity) {
            for (const ComponentEntry *entry : entries)
                if (!entry->has(gm, entity))
                    return;
            func(entity);
        });
    }

    void Game::registerView(sol::state &lua)
    {
        /**
         * @brief Where a gm:view loop is: shared by every copy sol makes
         * of its iterator.
         */
        struct ViewState {
            GameManager &gm;
            std::vector<const ComponentEntry *> entries;
            std::vector<Entity> entities;
            std::size_t next = 0;
        };

        lua["GameManager"]["view"] = [](GameManager &gm,
                                         const sol::table &names) {
            auto state = std::make_shared<ViewState>(
                ViewState {gm, resolveComponents(names), {}});

            forEachMatch(gm, state->entries, [&state](Entity entity) {
                state->entities.push_back(entity);
            });
            return [state](sol::this_state L) {
                std::vector<sol::object> values;

                while (state->next < state->entities.size()) {
                    Entity entity = state->entities[state->next++];

                    if (!std::ranges::all_of(state->entries,
                            [&](const ComponentEntry *entry) {
                                return entry->has(state->gm, entity);
                            }))
                        continue;
                    values.push_back(sol::make_object(L, entity));
                    for (const ComponentEntry *entry : state->entries)
                        values.push_back(entry->get(L, state->gm, entity));
                    return sol::as_returns(std::move(values));
                }
                values.push_back(sol::make_object(L, sol::lua_nil));
                return sol::as_returns(std::move(values));
            };
        };
    }

}
