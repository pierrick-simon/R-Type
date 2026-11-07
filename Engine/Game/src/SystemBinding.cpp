/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemBinding
*/

#include "SystemBinding.hpp"
#include "Exceptions.hpp"
#include "Scripts.hpp"

namespace RType::Game {

    void SystemBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["createSystem"] = sol::overload(
            [](GameManager &gm, const sol::protected_function &func) {
                checkNotUpdating(gm, "gm:createSystem");
                return gm.createSystem<>([func](GameManager &gm) {
                    Scripts::checkResult(func(gm));
                });
            },
            [](GameManager &gm,
                const sol::table &names,
                const sol::protected_function &func) {
                checkNotUpdating(gm, "gm:createSystem");
                std::vector<const ComponentBinding::Entry *> entries =
                    ComponentBinding::resolve(names);

                return gm.createSystem<>([entries, func](GameManager &gm) {
                    if (entries.empty())
                        Scripts::checkResult(func(gm));
                    else
                        runScriptSystem(gm, entries, func);
                });
            });
        lua["GameManager"]["addSystem"] = [](GameManager &gm,
                                              const std::string &name) {
            checkNotUpdating(gm, "gm:addSystem");
            auto iter = engineSystems.find(name);
            if (iter == engineSystems.end())
                throw UnknownSystem(name);
            return iter->second(gm);
        };
        lua["GameManager"]["destroySystem"] = &GameManager::destroySystem;
        lua["GameManager"]["hasSystem"] = &GameManager::hasSystem;
    }

    void SystemBinding::checkNotUpdating(
        const GameManager &gm, const std::string &function)
    {
        if (gm.isUpdating())
            throw SystemCreatedInSystem(function);
    }

    void SystemBinding::runScriptSystem(GameManager &gm,
        const std::vector<const ComponentBinding::Entry *> &entries,
        const sol::protected_function &func)
    {
        lua_State *L = func.lua_state();
        std::vector<sol::object> components(entries.size());

        ComponentBinding::forEachMatch(gm, entries, [&](Entity entity) {
            for (std::size_t i = 0; i < entries.size(); i++)
                components[i] = entries[i]->get(L, gm, entity);
            Scripts::checkResult(func(gm, entity, sol::as_args(components)));
        });
    }

} // namespace RType::Game
