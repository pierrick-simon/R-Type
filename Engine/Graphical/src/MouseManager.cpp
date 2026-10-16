/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Mouse
*/

#include "MouseManager.hpp"
#include <algorithm>
#include "Exceptions.hpp"

namespace RType {
    MouseButton::Value MouseButton::getValue(int keyCode)
    {
        auto tmp = _mouseButtonMap.find(keyCode);
        if (tmp == _mouseButtonMap.end())
            throw EngineException("Key not found in the key map");
        return tmp->second;
    }

    int MouseButton::getMouseButtonCode(Value value)
    {
        for (const auto &box : _mouseButtonMap) {
            if (box.second == value)
                return box.first;
        }
        throw EngineException("Key not found in the key map");
    }

    MouseManager::MouseManager()
    {
        static bool onlyOne = true;
        if (!onlyOne)
            throw EngineException("You can only have one Mouse Component");
    }

    void MouseManager::systemMouseManager(MouseManager &m)
    {
        m.buttonPressed.clear();
        m.buttonDown.clear();
        m.buttonReleased.clear();
        for (int button = MOUSE_BUTTON_LEFT; button <= MOUSE_BUTTON_BACK;
            button++) {
            MouseButton::Value v;
            try {
                v = MouseButton::getValue(button);
            } catch (const EngineException &) {
                continue;
            }
            if (raylib::Mouse::IsButtonPressed(button))
                m.buttonPressed.push_back(v);
            if (raylib::Mouse::IsButtonDown(button))
                m.buttonDown.push_back(v);
            if (raylib::Mouse::IsButtonReleased(button))
                m.buttonReleased.push_back(v);
        }
        m._wheel = raylib::Mouse::GetWheelMove();
        m._pos.x = float(raylib::Mouse::GetX());
        m._pos.y = float(raylib::Mouse::GetY());
    }

    const std::unordered_map<int, MouseButton::Value>
        MouseButton::_mouseButtonMap {
            {MOUSE_BUTTON_LEFT, Value::Left},
            {MOUSE_BUTTON_RIGHT, Value::Right},
            {MOUSE_BUTTON_MIDDLE, Value::Middle},
            {MOUSE_BUTTON_SIDE, Value::Side},
            {MOUSE_BUTTON_EXTRA, Value::Extra},
            {MOUSE_BUTTON_FORWARD, Value::Forward},
            {MOUSE_BUTTON_BACK, Value::Back},
        };
}; // namespace RType
