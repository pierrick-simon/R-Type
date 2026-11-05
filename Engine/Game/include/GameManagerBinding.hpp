/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManagerBinding
*/

#ifndef GAMEMANAGERBINDING_HPP_
#define GAMEMANAGERBINDING_HPP_

#include <sol/sol.hpp>

namespace RType::Game {

    /**
     * @brief Exposes GameManager and Entity to Lua.
     */
    class GameManagerBinding {
    public:
        GameManagerBinding() = delete;

        /**
         * @brief Exposes GameManager and Entity to Lua, with everything
         * scripts can do on them: entities here, then components, systems,
         * views and inputs through their own bindings. Called once, before
         * any script is loaded.
         */
        static void registerFunctions(sol::state &lua);
    };

} // namespace RType::Game

#endif /* !GAMEMANAGERBINDING_HPP_ */
