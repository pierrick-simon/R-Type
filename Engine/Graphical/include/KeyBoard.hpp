/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyBoard
*/

#ifndef KEYBOARD_HPP_
#define KEYBOARD_HPP_

    #include <raylib-cpp.hpp>
    #include <unordered_map>
    #include <unordered_set>

namespace RType {

    /**
     * @brief Key helper converting raylib key codes to RType values.
     */
    class Key {
    public:
        /**
         * @brief Keyboard keys.
         */
        enum class Value {
            A, //!< The A key
            B, //!< The B key
            C, //!< The C key
            D, //!< The D key
            E, //!< The E key
            F, //!< The F key
            G, //!< The G key
            H, //!< The H key
            I, //!< The I key
            J, //!< The J key
            K, //!< The K key
            L, //!< The L key
            M, //!< The M key
            N, //!< The N key
            O, //!< The O key
            P, //!< The P key
            Q, //!< The Q key
            R, //!< The R key
            S, //!< The S key
            T, //!< The T key
            U, //!< The U key
            V, //!< The V key
            W, //!< The W key
            X, //!< The X key
            Y, //!< The Y key
            Z, //!< The Z key
            Num0, //!< The 0 key
            Num1, //!< The 1 key
            Num2, //!< The 2 key
            Num3, //!< The 3 key
            Num4, //!< The 4 key
            Num5, //!< The 5 key
            Num6, //!< The 6 key
            Num7, //!< The 7 key
            Num8, //!< The 8 key
            Num9, //!< The 9 key
            Escape, //!< The Escape key
            LControl, //!< The left Control key
            LShift, //!< The left Shift key
            LAlt, //!< The left Alt key
            LSystem, //!< The left OS specific key: window (Windows and Linux),
                     //!< apple (macOS), ...
            RControl, //!< The right Control key
            RShift, //!< The right Shift key
            RAlt, //!< The right Alt key
            RSystem, //!< The right OS specific key: window (Windows and Linux),
                     //!< apple (macOS), ...
            Menu, //!< The Menu key
            LBracket, //!< The [ key
            RBracket, //!< The ] key
            Semicolon, //!< The ; key
            Comma, //!< The , key
            Period, //!< The . key
            Apostrophe, //!< The ' key
            Slash, //!< The / key
            Backslash, //!< The \ key
            Grave, //!< The ` key
            Equal, //!< The = key
            Hyphen, //!< The - key (hyphen)
            Space, //!< The Space key
            Enter, //!< The Enter/Return keys
            Backspace, //!< The Backspace key
            Tab, //!< The Tabulation key
            PageUp, //!< The Page up key
            PageDown, //!< The Page down key
            End, //!< The End key
            Home, //!< The Home key
            Insert, //!< The Insert key
            Delete, //!< The Delete key
            Add, //!< The + key
            Subtract, //!< The - key (minus, usually from numpad)
            Multiply, //!< The * key
            Divide, //!< The / key
            Left, //!< Left arrow
            Right, //!< Right arrow
            Up, //!< Up arrow
            Down, //!< Down arrow
            Numpad0, //!< The numpad 0 key
            Numpad1, //!< The numpad 1 key
            Numpad2, //!< The numpad 2 key
            Numpad3, //!< The numpad 3 key
            Numpad4, //!< The numpad 4 key
            Numpad5, //!< The numpad 5 key
            Numpad6, //!< The numpad 6 key
            Numpad7, //!< The numpad 7 key
            Numpad8, //!< The numpad 8 key
            Numpad9, //!< The numpad 9 key
            F1, //!< The F1 key
            F2, //!< The F2 key
            F3, //!< The F3 key
            F4, //!< The F4 key
            F5, //!< The F5 key
            F6, //!< The F6 key
            F7, //!< The F7 key
            F8, //!< The F8 key
            F9, //!< The F9 key
            F10, //!< The F10 key
            F11, //!< The F11 key
            F12, //!< The F12 key
            F13, //!< The F13 key
            F14, //!< The F14 key
            F15, //!< The F15 key
            Pause, //!< The Pause key

            KeyCount, //!< Keep last -- the total number of keyboard keys
        };

        /**
         * @brief Convert a raylib key code to a Value.
         *
         * @param keyCode The raylib key code.
         * @return The corresponding Value.
         *
         * @throw std::out_of_range If @p keyCode is not a known key
         *        (if the implementation uses at()).
         *
         * @warning Only raylib codes are accepted, not Value integers.
         */
        static Value getValue(int keyCode);

        /**
         * @brief Convert a Value to the raylib key code.
         *
         * @param value The key.
         * @return The corresponding raylib key code.
         *
         * @throw std::out_of_range If @p value has no mapping
         *        (if the implementation uses at()).
         *
         * @warning Value::KeyCount is not a real key and must not be passed.
         */
        static int getKeyCode(Value value);

    private:
        static const std::unordered_map<int, Value> _keyMap;
    };

    /**
     * @brief Keyboard state component.
     *
     * The state is refreshed once per frame by systemKeyBoard().
     */
    class KeyBoard {
    public:
        /**
         * @brief Create a keyboard with an empty state.
         */
        KeyBoard();

        std::vector<Key::Value> keyPressed;
        std::vector<Key::Value> keyDown;
        std::vector<Key::Value> keyReleased;

        /**
         * @brief System refreshing the keyboard state.
         *
         * @param k The keyboard to update.
         *
         * @warning Must be called once per frame, after the window has been
         *          created, otherwise the state will be stale.
         */
        static void systemKeyBoard(KeyBoard &k);

    private:
        std::unordered_set<int> _held;
    };

} // namespace RType

#endif /* !KEYBOARD_HPP_ */
