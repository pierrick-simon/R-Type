/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyBindings
*/

#include "KeyBindings.hpp"

#include <algorithm>
#include <vector>

namespace RType::Client {

    namespace {

        /** @brief Add "from acts as to" to the right map of the Keybind. */
        void addBinding(Keybind &bind, const Input &from, const Input &to)
        {
            const auto key = [](const Input &i) {
                return static_cast<Key::Value>(i.code);
            };
            const auto button = [](const Input &i) {
                return static_cast<MouseButton::Value>(i.code);
            };

            if (!from.mouse && !to.mouse)
                bind._keyToKey[key(from)] = key(to);
            else if (!from.mouse)
                bind._keyToMouseButton[key(from)] = button(to);
            else if (!to.mouse)
                bind._mouseButtonToKey[button(from)] = key(to);
            else
                bind._mouseButtonToMouseButton[button(from)] = button(to);
        }

    } // namespace

    void KeyBindings::fillKeybind(Keybind &bind)
    {
        std::vector<Input> freed;
        std::vector<Input> orphans;

        bind._keyToKey.clear();
        bind._keyToMouseButton.clear();
        bind._mouseButtonToKey.clear();
        bind._mouseButtonToMouseButton.clear();
        for (std::size_t i = 0; i < ACTION_COUNT; i++) {
            const Input &input = _keyBindings.at(i);

            if (input != DEFAULT_KEYS.at(i))
                addBinding(bind, input, DEFAULT_KEYS.at(i));
            if (std::ranges::find(DEFAULT_KEYS, input) == DEFAULT_KEYS.end())
                freed.push_back(input);
        }
        for (const Input &input : DEFAULT_KEYS)
            if (std::ranges::find(_keyBindings, input) == _keyBindings.end())
                orphans.push_back(input);
        for (std::size_t i = 0; i < orphans.size() && i < freed.size(); i++)
            addBinding(bind, orphans.at(i), freed.at(i));
    }

    void KeyBindings::bind(Action action, Input input)
    {
        const auto index = static_cast<std::size_t>(action);
        const auto other = std::ranges::find(_keyBindings, input);

        if (other != _keyBindings.end())
            *other = _keyBindings.at(index);
        _keyBindings.at(index) = input;
    }

    std::string KeyBindings::keyName(Key::Value key)
    {
        const auto index = static_cast<std::size_t>(key);

        if (index >= KEY_NAMES.size())
            return "KEY " + std::to_string(index);
        return KEY_NAMES.at(index);
    }

    std::string KeyBindings::inputName(const Input &input)
    {
        static constexpr std::array<const char *,
            static_cast<std::size_t>(MouseButton::Value::MouseButtonCount)>
            BUTTON_NAMES = {"MOUSE LEFT",
                "MOUSE RIGHT",
                "MOUSE MIDDLE",
                "MOUSE SIDE",
                "MOUSE EXTRA",
                "MOUSE FORWARD",
                "MOUSE BACK"};

        if (!input.mouse)
            return keyName(static_cast<Key::Value>(input.code));
        if (input.code < 0 ||
            static_cast<std::size_t>(input.code) >= BUTTON_NAMES.size())
            return "MOUSE " + std::to_string(input.code);
        return BUTTON_NAMES.at(static_cast<std::size_t>(input.code));
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

} // namespace RType::Client
