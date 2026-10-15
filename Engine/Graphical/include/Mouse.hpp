/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Mouse
*/

#ifndef MOUSE_HPP_
#define MOUSE_HPP_

#include <raylib-cpp.hpp>
#include <unordered_map>
#include <unordered_set>
#include "Position2D.hpp"

namespace RType {

    class MouseButton {
    public:
        enum class Value {
            Left, //!< The Left Mouse Button
            Right, //!< The Right Mouse Button
            Middle, //!< The Middle Mouse Button
            Side, //!< The Side Mouse Button
            Extra, //!< The Extra Mouse Button
            Forward, //!< The Forward Mouse Button
            Back, //!< The Back Mouse Button
        };

        static Value getValue(int keyCode);
        static int getKeyCode(Value value);

    private:
        static const std::unordered_map<int, Value> _mouseButtonMap;
    };

    class Mouse {
    public:
        Mouse();

        std::vector<MouseButton::Value> buttonPressed;
        std::vector<MouseButton::Value> buttonDown;

        static void systemMouse(Mouse &m);

        Position2D getPos() const
        {
            return _pos;
        }

        float getWheel() const
        {
            return _wheel;
        }

    private:
        std::unordered_set<int> _held;
        Position2D _pos;
        float _wheel = 0.f;
    };

} // namespace RType

#endif /* !MOUSE_HPP_ */
