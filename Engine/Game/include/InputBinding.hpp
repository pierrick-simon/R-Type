/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** InputBinding
*/

#ifndef INPUTBINDING_HPP_
#define INPUTBINDING_HPP_

#include <algorithm>
#include <vector>
#include <sol/sol.hpp>

namespace RType::Game {

    /**
     * @brief Lets scripts read the keyboard of the Keyboard module and the
     * mouse of the Mouse module. They are not components: scripts cannot
     * create other ones.
     */
    class InputBinding {
    public:
        InputBinding() = delete;

        /**
         * @brief Lets scripts read the keyboard and the mouse:
         * - gm:keyboard() returns the Keyboard module's KeyboardManager;
         * - kb:isPressed(key), kb:isDown(key) and kb:isReleased(key) tell
         *   whether a key was pressed during this frame, is held, or was
         *   released during this frame;
         * - the Key table holds the keys: Key.Space, Key.Left...
         * - gm:mouse() returns the Mouse module's MouseManager;
         * - mouse:isPressed(button), mouse:isDown(button) and
         *   mouse:isReleased(button) do the same for the buttons, and
         *   mouse.x, mouse.y and mouse.wheel give the cursor's position and
         *   the wheel, read-only;
         * - the MouseButton table holds the buttons: MouseButton.Left...
         * @throw ModuleNotLoaded If gm:keyboard or gm:mouse is called
         * without the Keyboard or the Mouse module.
         */
        static void registerFunctions(sol::state &lua);

    private:
        /**
         * @brief Exposes a list member, such as KeyboardManager::keyDown,
         * to Lua as a method telling whether a value is in it:
         * kb:isDown(Key.Space).
         */
        template <typename Manager, typename Value>
        static auto contains(std::vector<Value> Manager::*list);
    };

    template <typename Manager, typename Value>
    auto InputBinding::contains(std::vector<Value> Manager::*list)
    {
        return [list](const Manager &m, Value value) {
            return std::ranges::find(m.*list, value) != (m.*list).end();
        };
    }

} // namespace RType::Game

#endif /* !INPUTBINDING_HPP_ */
