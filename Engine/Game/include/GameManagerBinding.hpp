/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManagerBinding
*/

#ifndef GAMEMANAGERBINDING_HPP_
#define GAMEMANAGERBINDING_HPP_

#include <sol/sol.hpp>

#include "Exceptions.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Exposes GameManager and Entity to Lua.
     */
    class GameManagerBinding {
    public:
        GameManagerBinding() = delete;

        /**
         * @brief Exposes GameManager and Entity to Lua, with everything
         * scripts can do on them: entities and gm:stop(), which ends the
         * game after the current frame, here, then components, systems,
         * views, inputs and time through their own bindings. Called once, before
         * any script is loaded.
         */
        static void registerFunctions(sol::state &lua);

        /**
         * @brief Finds the component a module created, such as the
         * KeyboardManager of the Keyboard module, for gm:keyboard,
         * gm:mouse and gm:elapsed.
         * @param function The Lua function the script called, for the
         * error message.
         * @param module The module that creates the component.
         * @throw ModuleNotLoaded If no entity has the component.
         */
        template <typename Component>
        static Component &findModuleComponent(
            GameManager &gm, const char *function, const char *module);
    };

    template <typename Component>
    Component &GameManagerBinding::findModuleComponent(
        GameManager &gm, const char *function, const char *module)
    {
        Component *found = nullptr;

        gm.view<Component>().each([&found](Entity, Component &c) {
            if (!found)
                found = &c;
        });
        if (!found)
            throw ModuleNotLoaded(function, module);
        return *found;
    }

} // namespace RType::Game

#endif /* !GAMEMANAGERBINDING_HPP_ */
