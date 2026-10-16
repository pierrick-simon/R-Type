/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyBind
*/

#ifndef KEYBIND_HPP_
#define KEYBIND_HPP_

#include <algorithm>
#include <unordered_map>
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"

namespace RType {

    /**
     * @brief Input rebinding component.
     *
     * Each map translates an input (the key) into another input (the value).
     * Inputs without a binding are left untouched. A bound input is removed
     * from its original list and added to the list of its target.
     */
    class KeyBind {
    public:
        /** @brief Keyboard key -> another keyboard key. */
        std::unordered_map<Key::Value, Key::Value> _keyToKey;

        /** @brief Mouse button -> another mouse button. */
        std::unordered_map<MouseButton::Value, MouseButton::Value>
            _mouseButtonToMouseButton;

        /** @brief Keyboard key -> mouse button (the key acts as a click). */
        std::unordered_map<Key::Value, MouseButton::Value> _keyToMouseButton;

        /** @brief Mouse button -> keyboard key (the click acts as a key). */
        std::unordered_map<MouseButton::Value, Key::Value> _mouseButtonToKey;

        /**
         * @brief System applying the bindings to the current input state.
         *
         * Rewrites the pressed, down and released lists of both managers
         * according to the bind maps. Bindings are applied once and do not
         * chain (A->B and B->C turns A into B, not C).
         *
         * @param bind The bindings to apply.
         * @param mouse The mouse manager to remap.
         * @param keyboard The keyboard manager to remap.
         *
         * @warning Must be called every frame, after
         *          MouseManager::systemMouseManager() and
         *          KeyboardManager::systemKeyboardManager(), since those
         *          systems rebuild the lists from the raw input each frame.
         */
        static void systemUpdateKeyBind(const KeyBind &bind,
            MouseManager &mouse, KeyboardManager &keyboard);

    private:
        /**
         * @brief Remap one state list of both devices.
         *
         * Replaces every input found in @p keys and @p buttons by its bound
         * target, according to the bind maps of @p bind. Inputs without a
         * binding are kept as they are. Bindings are applied once and do not
         * chain (A->B and B->C turns A into B, not C). The result contains no
         * duplicates.
         *
         * @param bind The bindings to apply.
         * @param[in,out] keys The keyboard list (pressed, down or released) to
         *        remap. Receives the remapped keys, including those coming from
         *        mouse buttons bound to a key.
         * @param[in,out] buttons The mouse button list (pressed, down or
         * released) to remap. Receives the remapped buttons, including those
         * coming from keys bound to a mouse button.
         *
         * @note @p keys and @p buttons must be lists of the same kind (for
         * example both "pressed"), since inputs can move from one list to the
         * other.
         */
        static void remap(const KeyBind &bind, std::vector<Key::Value> &keys,
            std::vector<MouseButton::Value> &buttons);

        /**
         * @brief Add a value to a list only if it is not already present.
         *
         * @tparam T The type of the elements stored in the list.
         * @param[in,out] list The list to add the value to.
         * @param value The value to add.
         *
         * @note The search is linear, which is fine for the small input lists
         *       handled each frame.
         */
        template<typename T>
        static void addUnique(std::vector<T> &list, T value)
        {
            if (std::find(list.begin(), list.end(), value) == list.end())
                list.push_back(value);
        }
    };

} // namespace RType

#endif /* !KEYBIND_HPP_ */
