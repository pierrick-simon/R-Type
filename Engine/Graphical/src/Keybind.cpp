/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Keybind
*/

#include "Keybind.hpp"
#include <utility>
#include <vector>

namespace RType {
    void Keybind::remap(const Keybind &bind, std::vector<Key::Value> &keys,
        std::vector<MouseButton::Value> &buttons)
    {
        std::vector<Key::Value> newKeys;
        std::vector<MouseButton::Value> newButtons;

        for (Key::Value key : keys) {
            auto toKey = bind._keyToKey.find(key);
            if (toKey != bind._keyToKey.end()) {
                addUnique(newKeys, toKey->second);
                continue;
            }
            auto toButton = bind._keyToMouseButton.find(key);
            if (toButton != bind._keyToMouseButton.end()) {
                addUnique(newButtons, toButton->second);
                continue;
            }
            addUnique(newKeys, key);
        }

        for (MouseButton::Value button : buttons) {
            auto toKey = bind._mouseButtonToKey.find(button);
            if (toKey != bind._mouseButtonToKey.end()) {
                addUnique(newKeys, toKey->second);
                continue;
            }
            auto toButton = bind._mouseButtonToMouseButton.find(button);
            if (toButton != bind._mouseButtonToMouseButton.end()) {
                addUnique(newButtons, toButton->second);
                continue;
            }
            addUnique(newButtons, button);
        }

        keys = std::move(newKeys);
        buttons = std::move(newButtons);
    }

    void Keybind::systemUpdateKeybind(
        const Keybind &bind, MouseManager &mouse, KeyboardManager &keyboard)
    {
        remap(bind, keyboard.keyPressed, mouse.buttonPressed);
        remap(bind, keyboard.keyDown, mouse.buttonDown);
        remap(bind, keyboard.keyReleased, mouse.buttonReleased);
    }
} // namespace RType
