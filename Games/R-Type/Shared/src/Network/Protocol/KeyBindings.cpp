/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyBindings
*/

#include "KeyBindings.hpp"

#include <algorithm>

namespace RType {

    namespace {

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

    } // namespace

    std::optional<Key::Value> KeyBindings::translate(Key::Value key)
    {
        for (std::size_t i = 0; i < ACTION_COUNT; i++)
            if (_keyBindings.at(i) == key)
                return DEFAULT_KEYS.at(i);
        if (std::ranges::find(DEFAULT_KEYS, key) != DEFAULT_KEYS.end())
            return std::nullopt;
        return key;
    }

    void KeyBindings::bind(Action action, Key::Value key)
    {
        const auto index = static_cast<std::size_t>(action);
        const auto other = std::ranges::find(_keyBindings, key);

        if (other != _keyBindings.end())
            *other = _keyBindings.at(index);
        _keyBindings.at(index) = key;
    }

    std::string KeyBindings::keyName(Key::Value key)
    {
        const auto index = static_cast<std::size_t>(key);

        if (index >= KEY_NAMES.size())
            return "KEY " + std::to_string(index);
        return KEY_NAMES.at(index);
    }

    std::string KeyBindings::actionName(Action action)
    {
        switch (action) {
            case Action::Up:
                return "Up";
            case Action::Left:
                return "Left";
            case Action::Down:
                return "Down";
            case Action::Right:
                return "Right";
            case Action::Shoot:
                return "Shoot";
            default:
                return "";
        }
    }

} // namespace RType
