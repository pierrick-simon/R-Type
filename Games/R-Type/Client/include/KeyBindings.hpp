/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyBindings
*/

#ifndef KEYBINDINGS_HPP_
#define KEYBINDINGS_HPP_

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include "Keybind.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"

namespace RType::Client {

    constexpr std::array<const char *,
        static_cast<std::size_t>(Key::Value::KeyCount)>
        KEY_NAMES = {"A",
            "B",
            "C",
            "D",
            "E",
            "F",
            "G",
            "H",
            "I",
            "J",
            "K",
            "L",
            "M",
            "N",
            "O",
            "P",
            "Q",
            "R",
            "S",
            "T",
            "U",
            "V",
            "W",
            "X",
            "Y",
            "Z",
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8",
            "9",
            "ESCAPE",
            "L CTRL",
            "L SHIFT",
            "L ALT",
            "L SYSTEM",
            "R CTRL",
            "R SHIFT",
            "R ALT",
            "R SYSTEM",
            "MENU",
            "L BRACKET",
            "R BRACKET",
            "SEMICOLON",
            "COMMA",
            "PERIOD",
            "APOSTROPHE",
            "SLASH",
            "BACKSLASH",
            "GRAVE",
            "EQUAL",
            "HYPHEN",
            "SPACE",
            "ENTER",
            "BACKSPACE",
            "TAB",
            "PAGE UP",
            "PAGE DOWN",
            "END",
            "HOME",
            "INSERT",
            "DELETE",
            "ADD",
            "SUBTRACT",
            "MULTIPLY",
            "DIVIDE",
            "LEFT",
            "RIGHT",
            "UP",
            "DOWN",
            "NUMPAD 0",
            "NUMPAD 1",
            "NUMPAD 2",
            "NUMPAD 3",
            "NUMPAD 4",
            "NUMPAD 5",
            "NUMPAD 6",
            "NUMPAD 7",
            "NUMPAD 8",
            "NUMPAD 9",
            "F1",
            "F2",
            "F3",
            "F4",
            "F5",
            "F6",
            "F7",
            "F8",
            "F9",
            "F10",
            "F11",
            "F12",
            "F13",
            "F14",
            "F15",
            "PAUSE"};

    /** @brief Game actions the player can bind to a key. */
    enum class Action : std::size_t { Up, Left, Down, Right, Shoot, Count };

    inline constexpr std::size_t ACTION_COUNT =
        static_cast<std::size_t>(Action::Count);

    /** @brief An input an action can be bound to: a key or a mouse button. */
    struct Input {
        bool mouse = false; //!< true: code is a MouseButton::Value
        int code = 0; //!< Key::Value or MouseButton::Value

        static constexpr Input key(Key::Value value)
        {
            return {false, static_cast<int>(value)};
        }

        static constexpr Input button(MouseButton::Value value)
        {
            return {true, static_cast<int>(value)};
        }

        bool operator==(const Input &) const = default;
    };

    /** @brief Key of each action expected by the server (ZQSD + Space). */
    inline constexpr std::array<Input, ACTION_COUNT> DEFAULT_KEYS = {
        Input::key(Key::Value::Z),
        Input::key(Key::Value::Q),
        Input::key(Key::Value::S),
        Input::key(Key::Value::D),
        Input::key(Key::Value::Space)};

    /**
     * @brief Input chosen by the player for each action (client side).
     *
     * The server only knows the default keys: the engine Keybind of the
     * client turns the chosen input into the default key while playing
     * (see KeyBindings::fillKeybind).
     */
    inline std::array<Input, ACTION_COUNT> _keyBindings = DEFAULT_KEYS;

    class KeyBindings {
    public:
        /**
         * @brief Fill the engine Keybind with the current bindings.
         *
         * The server only knows the default keys (Z Q S D Space), so the
         * Keybind turns every bound key into the default key of its action
         * (Up arrow -> Z if Up is bound to "up"). A default key that is no
         * longer bound is turned into one of the freed keys, which the
         * server ignores: Z no longer moves the ship. The mapping is a
         * permutation, so no two keys end up on the same action.
         *
         * @param bind The Keybind of the client's input entity.
         */
        static void fillKeybind(Keybind &bind);

        /** @brief Binds @p key to @p action. If another action had this key,
         *         the two actions swap their keys. */
        static void bind(Action action, Input input);

        /** @brief Name of a key, with letters, digits and spaces only (the
         *         font of the game has no other character). */
        static std::string keyName(Key::Value key);

        /** @brief Label of an input: the key name, or "MOUSE LEFT"... */
        static std::string inputName(const Input &input);

        static std::string actionName(Action action);
    };

} // namespace RType::Client

#endif /* !KEYBINDINGS_HPP_ */
