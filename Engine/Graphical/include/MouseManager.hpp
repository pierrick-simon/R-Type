/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MouseManager
*/

#ifndef MOUSMANAGER_HPP_
#define MOUSMANAGER_HPP_

#include <raylib-cpp.hpp>
#include <unordered_map>
#include <unordered_set>
#include "Position2D.hpp"

namespace RType {

    /**
     * @brief Mouse button helper converting raylib codes to RType values.
     */
    class MouseButton {
    public:
        /**
         * @brief Mouse buttons.
         */
        enum class Value {
            Left, //!< The Left Mouse Button
            Right, //!< The Right Mouse Button
            Middle, //!< The Middle Mouse Button
            Side, //!< The Side Mouse Button
            Extra, //!< The Extra Mouse Button
            Forward, //!< The Forward Mouse Button
            Back, //!< The Back Mouse Button
        };

        /**
         * @brief Convert a raylib mouse button code to a Value.
         *
         * @param keyCode The raylib mouse button code.
         * @return The corresponding Value.
         *
         * @throw std::out_of_range If @p keyCode is not a known button
         *        (if the implementation uses at()).
         *
         * @warning Only raylib codes are accepted, not Value integers.
         */
        static Value getValue(int keyCode);

        /**
         * @brief Convert a Value to the raylib mouse button code.
         *
         * @param value The mouse button.
         * @return The corresponding raylib code.
         *
         * @throw std::out_of_range If @p value has no mapping
         *        (if the implementation uses at()).
         */
        static int getMouseButtonCode(Value value);

    private:
        static const std::unordered_map<int, Value> _mouseButtonMap;
    };

    /**
     * @brief MouseManager state component.
     *
     * The state is refreshed once per frame by systemMouse().
     */
    class MouseManager {
    public:
        std::vector<MouseButton::Value> buttonPressed;
        std::vector<MouseButton::Value> buttonDown;
        std::vector<MouseButton::Value> buttonReleased;

        /**
         * @brief System refreshing the mouse state.
         *
         * @param m The mouse to update.
         *
         * @warning Must be called once per frame, after the window has been
         *          created, otherwise the state will be stale.
         */
        static void systemMouseManager(MouseManager &m);

        /**
         * @brief Get the mouse position.
         *
         * @return The position of the cursor in the window, in pixels, as of
         *         the last call to systemMouse().
         */
        Position2D getPos() const
        {
            return _pos;
        }

        /**
         * @brief Get the mouse wheel movement.
         *
         * @return The wheel movement of the last frame (positive = up).
         */
        float getWheel() const
        {
            return _wheel;
        }

    private:
        std::unordered_set<int> _held;
        Position2D _pos;
        float _wheel = 0.f;
    };

    /**
     * @brief Mouse component.
     *
     * Is link to the MouseManager to get the update input.
     */
    struct Mouse {
        MouseManager &mouse;
    };

} // namespace RType

#endif /* !MOUSEMANAGER_HPP_ */
