/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ViewBinding
*/

#include <algorithm>
#include <memory>
#include <vector>

#include "ViewBinding.hpp"

namespace RType::Game {

    void ViewBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["view"] = [](GameManager &gm,
                                         const sol::table &names) {
            auto state = std::make_shared<State>(
                State {gm, ComponentBinding::resolve(names), {}});

            ComponentBinding::forEachMatch(gm,
                state->entries,
                [&state](Entity entity) { state->entities.push_back(entity); });
            return [state](sol::this_state L) {
                std::vector<sol::object> values;

                while (state->next < state->entities.size()) {
                    Entity entity = state->entities[state->next++];

                    if (!std::ranges::all_of(state->entries,
                            [&](const ComponentBinding::Entry *entry) {
                                return entry->has(state->gm, entity);
                            }))
                        continue;
                    values.push_back(sol::make_object(L, entity));
                    for (const ComponentBinding::Entry *entry : state->entries)
                        values.push_back(entry->get(L, state->gm, entity));
                    return sol::as_returns(std::move(values));
                }
                values.push_back(sol::make_object(L, sol::lua_nil));
                return sol::as_returns(std::move(values));
            };
        };
    }

} // namespace RType::Game
