/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemBinding
*/

#ifndef SYSTEMBINDING_HPP_
#define SYSTEMBINDING_HPP_

#include <functional>
#include <sol/sol.hpp>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ComponentBinding.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Lets scripts create their own systems, add the engine's ones
     * by name, and stop them.
     */
    class SystemBinding {
    public:
        SystemBinding() = delete;

        /**
         * @brief Lets scripts manage systems:
         * - gm:createSystem(func): func(gm) is called once each update();
         * - gm:createSystem({"Position2D", "Velocity2D"}, func):
         *   func(gm, entity, position, velocity) is called each update()
         *   for every entity that has all the components, given in the
         *   list's order. They are the real components: writing their
         *   fields changes them in C++ too. An empty list is the same as
         *   the first form;
         * - gm:addSystem("DrawSprite2D") adds one of the engine's systems.
         *   The name is the C++ function's without its "system" prefix;
         * - gm:destroySystem(id) and gm:hasSystem(id), with the id returned
         *   by gm:createSystem or gm:addSystem.
         *
         * The names are resolved when the system is created, so that a
         * wrong one stops the script's start.
         * @throw UnknownComponent If a name is not a component.
         * @throw InvalidComponentList If the list holds something else than
         * a name, or the same name twice.
         * @throw UnknownSystem If the name is not an engine system.
         * @throw SystemCreatedInSystem If gm:createSystem or gm:addSystem is
         * called from a system, instead of from start.
         */
        static void registerFunctions(sol::state &lua);

    private:
        /**
         * @brief Creates one engine system, for gm:addSystem.
         */
        using Factory = std::function<SystemId(GameManager &)>;

        /**
         * @brief Wraps an engine system into a factory that creates it, with
         * the components read from the system's own parameters.
         */
        template<typename... Components>
        static Factory makeSystem(void (*system)(Components...));

        /**
         * @brief Same, for a system that also takes the GameManager first.
         */
        template<typename... Components>
        static Factory makeSystem(void (*system)(GameManager &, Components...));

        /**
         * @brief Every engine system scripts can add, by name. Defined in
         * EngineBindings.cpp, the only file to edit to make a new system
         * usable from Lua.
         */
        static const std::unordered_map<std::string, Factory> engineSystems;

        /**
         * @brief Refuses to create a system while the systems are running:
         * it would run in the current frame, and one created at each frame
         * would make the systems pile up.
         * @param function The Lua function the script called, for the
         * error message.
         * @throw SystemCreatedInSystem If called during update().
         */
        static void checkNotUpdating(
            const GameManager &gm, const std::string &function);

        /**
         * @brief Calls a script's system on every entity that has all the
         * given components, with the entity then the components.
         */
        static void runScriptSystem(GameManager &gm,
            const std::vector<const ComponentBinding::Entry *> &entries,
            const sol::protected_function &func);
    };

    template<typename... Components>
    SystemBinding::Factory SystemBinding::makeSystem(
        void (*system)(Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

    template<typename... Components>
    SystemBinding::Factory SystemBinding::makeSystem(
        void (*system)(GameManager &, Components...))
    {
        return [system](GameManager &gm) {
            return gm.createSystem<std::remove_cvref_t<Components>...>(system);
        };
    }

} // namespace RType::Game

#endif /* !SYSTEMBINDING_HPP_ */
