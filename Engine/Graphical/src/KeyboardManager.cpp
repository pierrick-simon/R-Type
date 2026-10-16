/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Keyboard
*/

#include "KeyboardManager.hpp"
#include <algorithm>
#include "Exceptions.hpp"

namespace RType {
    Key::Value Key::getValue(int keyCode, Key::Layout layout)
    {
        auto keymap = _layout.find(layout);
        if (keymap == _layout.end())
            throw EngineException("Layout not found");
        auto tmp = keymap->second.find(keyCode);
        if (tmp == keymap->second.end())
            throw EngineException("Key not found in the key map");
        return tmp->second;
    }

    int Key::getKeyCode(Value value, Key::Layout layout)
    {
        auto keymap = _layout.find(layout);
        if (keymap == _layout.end())
            throw EngineException("Layout not found");
        for (const auto &box : keymap->second) {
            if (box.second == value)
                return box.first;
        }
        throw EngineException("Key not found in the key map");
    }

    KeyboardManager::KeyboardManager(Key::Layout layout) :
        layout(layout)
    {
        static bool onlyOne = true;
        if (!onlyOne)
            throw EngineException("You can only have one Keyboard Component");
    }

    void KeyboardManager::systemKeyboardManager(KeyboardManager &k)
    {
        k.keyPressed.clear();
        k.keyDown.clear();
        k.keyReleased.clear();

        for (auto it = k._held.begin(); it != k._held.end();) {
            if (!raylib::Keyboard::IsKeyDown(*it)) {
                Key::Value v = Key::getValue(*it, k.layout);
                if (std::find(k.keyReleased.begin(), k.keyReleased.end(), v) ==
                    k.keyReleased.end())
                    k.keyReleased.push_back(v);
                it = k._held.erase(it);
            } else {
                ++it;
            }
        }
        for (int key = raylib::Keyboard::GetKeyPressed(); key != KEY_NULL;
            key = raylib::Keyboard::GetKeyPressed()) {
            try {
                k.keyPressed.push_back(Key::getValue(key, k.layout));
            } catch (const EngineException &) {
                continue;
            }
            if (raylib::Keyboard::IsKeyDown(key))
                k._held.insert(key);
        }
        for (int key : k._held) {
            Key::Value v = Key::getValue(key, k.layout);
            if (std::find(k.keyDown.begin(), k.keyDown.end(), v) ==
                k.keyDown.end())
                k.keyDown.push_back(v);
        }
    }

    const std::unordered_map<int, Key::Value> Key::_keyMapAzerty {
        // Letters (AZERTY)
        {KEY_Q, Value::A}, // key labeled A sits at the QWERTY Q position
        {KEY_B, Value::B},
        {KEY_C, Value::C},
        {KEY_D, Value::D},
        {KEY_E, Value::E},
        {KEY_F, Value::F},
        {KEY_G, Value::G},
        {KEY_H, Value::H},
        {KEY_I, Value::I},
        {KEY_J, Value::J},
        {KEY_K, Value::K},
        {KEY_L, Value::L},
        {KEY_SEMICOLON, Value::M},
        {KEY_N, Value::N},
        {KEY_O, Value::O},
        {KEY_P, Value::P},
        {KEY_A, Value::Q}, // key labeled Q sits at the QWERTY A position
        {KEY_R, Value::R},
        {KEY_S, Value::S},
        {KEY_T, Value::T},
        {KEY_U, Value::U},
        {KEY_V, Value::V},
        {KEY_Z, Value::W}, // key labeled W sits at the QWERTY Z position
        {KEY_X, Value::X},
        {KEY_Y, Value::Y},
        {KEY_W, Value::Z}, // key labeled Z sits at the QWERTY W position

        // Number row (digits need Shift on AZERTY)
        {KEY_ONE, Value::Num1},
        {KEY_TWO, Value::Num2},
        {KEY_THREE, Value::Num3},
        {KEY_FOUR, Value::Num4},
        {KEY_FIVE, Value::Num5},
        {KEY_SIX, Value::Num6},
        {KEY_SEVEN, Value::Num7},
        {KEY_EIGHT, Value::Num8},
        {KEY_NINE, Value::Num9},
        {KEY_ZERO, Value::Num0},

        // Punctuation
        {KEY_M, Value::Comma}, // , is where M is on QWERTY
        {KEY_COMMA, Value::Semicolon}, // ; is where , is on QWERTY
        {KEY_PERIOD, Value::Period}, // physical key is ':' (and '.' is Shift+;)
        {KEY_SLASH, Value::Slash}, // physical key is '!'
        {KEY_APOSTROPHE, Value::Apostrophe}, // physical key is 'ù'
        {KEY_LEFT_BRACKET, Value::LBracket}, // physical key is '^'
        {KEY_RIGHT_BRACKET, Value::RBracket}, // physical key is '$'
        {KEY_BACKSLASH, Value::Backslash}, // physical key is '*'
        {KEY_GRAVE, Value::Grave}, // physical key is '²'
        {KEY_EQUAL, Value::Equal},
        {KEY_MINUS, Value::Hyphen}, // physical key is ')'

        // Control keys
        {KEY_ENTER, Value::Enter},
        {KEY_ESCAPE, Value::Escape},
        {KEY_BACKSPACE, Value::Backspace},
        {KEY_TAB, Value::Tab},
        {KEY_SPACE, Value::Space},
        {KEY_LEFT_CONTROL, Value::LControl},
        {KEY_RIGHT_CONTROL, Value::RControl},
        {KEY_LEFT_SHIFT, Value::LShift},
        {KEY_RIGHT_SHIFT, Value::RShift},
        {KEY_LEFT_ALT, Value::LAlt},
        {KEY_RIGHT_ALT, Value::RAlt}, // AltGr on AZERTY
        {KEY_LEFT_SUPER, Value::LSystem},
        {KEY_RIGHT_SUPER, Value::RSystem},
        {KEY_KB_MENU, Value::Menu},

        // Function / navigation
        {KEY_F1, Value::F1},
        {KEY_F2, Value::F2},
        {KEY_F3, Value::F3},
        {KEY_F4, Value::F4},
        {KEY_F5, Value::F5},
        {KEY_F6, Value::F6},
        {KEY_F7, Value::F7},
        {KEY_F8, Value::F8},
        {KEY_F9, Value::F9},
        {KEY_F10, Value::F10},
        {KEY_F11, Value::F11},
        {KEY_F12, Value::F12},
        {KEY_PAUSE, Value::Pause},
        {KEY_INSERT, Value::Insert},
        {KEY_HOME, Value::Home},
        {KEY_PAGE_UP, Value::PageUp},
        {KEY_DELETE, Value::Delete},
        {KEY_END, Value::End},
        {KEY_PAGE_DOWN, Value::PageDown},
        {KEY_RIGHT, Value::Right},
        {KEY_LEFT, Value::Left},
        {KEY_DOWN, Value::Down},
        {KEY_UP, Value::Up},

        // Numpad (same on both layouts)
        {KEY_KP_DIVIDE, Value::Divide},
        {KEY_KP_MULTIPLY, Value::Multiply},
        {KEY_KP_SUBTRACT, Value::Subtract},
        {KEY_KP_ADD, Value::Add},
        {KEY_KP_EQUAL, Value::Equal}, // see note on duplicate Values below
        {KEY_KP_0, Value::Numpad0},
        {KEY_KP_1, Value::Numpad1},
        {KEY_KP_2, Value::Numpad2},
        {KEY_KP_3, Value::Numpad3},
        {KEY_KP_4, Value::Numpad4},
        {KEY_KP_5, Value::Numpad5},
        {KEY_KP_6, Value::Numpad6},
        {KEY_KP_7, Value::Numpad7},
        {KEY_KP_8, Value::Numpad8},
        {KEY_KP_9, Value::Numpad9},
    };

    const std::unordered_map<int, Key::Value> Key::_keyMapQwerty {
        {KEY_A, Value::A},
        {KEY_B, Value::B},
        {KEY_C, Value::C},
        {KEY_D, Value::D},
        {KEY_E, Value::E},
        {KEY_F, Value::F},
        {KEY_G, Value::G},
        {KEY_H, Value::H},
        {KEY_I, Value::I},
        {KEY_J, Value::J},
        {KEY_K, Value::K},
        {KEY_L, Value::L},
        {KEY_M, Value::M},
        {KEY_N, Value::N},
        {KEY_O, Value::O},
        {KEY_P, Value::P},
        {KEY_Q, Value::Q},
        {KEY_R, Value::R},
        {KEY_S, Value::S},
        {KEY_T, Value::T},
        {KEY_U, Value::U},
        {KEY_V, Value::V},
        {KEY_W, Value::W},
        {KEY_X, Value::X},
        {KEY_Y, Value::Y},
        {KEY_Z, Value::Z},
        {KEY_ONE, Value::Num1},
        {KEY_TWO, Value::Num2},
        {KEY_THREE, Value::Num3},
        {KEY_FOUR, Value::Num4},
        {KEY_FIVE, Value::Num5},
        {KEY_SIX, Value::Num6},
        {KEY_SEVEN, Value::Num7},
        {KEY_EIGHT, Value::Num8},
        {KEY_NINE, Value::Num9},
        {KEY_ZERO, Value::Num0},
        {KEY_ENTER, Value::Enter},
        {KEY_ESCAPE, Value::Escape},
        {KEY_BACKSPACE, Value::Backspace},
        {KEY_TAB, Value::Tab},
        {KEY_SPACE, Value::Space},
        {KEY_EQUAL, Value::Equal},
        {KEY_KP_EQUAL, Value::Equal},
        {KEY_LEFT_BRACKET, Value::LBracket},
        {KEY_RIGHT_BRACKET, Value::RBracket},
        {KEY_BACKSLASH, Value::Backslash},
        {KEY_SEMICOLON, Value::Semicolon},
        {KEY_APOSTROPHE, Value::Apostrophe},
        {KEY_COMMA, Value::Comma},
        {KEY_PERIOD, Value::Period},
        {KEY_SLASH, Value::Slash},
        {KEY_F1, Value::F1},
        {KEY_F2, Value::F2},
        {KEY_F3, Value::F3},
        {KEY_F4, Value::F4},
        {KEY_F5, Value::F5},
        {KEY_F6, Value::F6},
        {KEY_F7, Value::F7},
        {KEY_F8, Value::F8},
        {KEY_F9, Value::F9},
        {KEY_F10, Value::F10},
        {KEY_F11, Value::F11},
        {KEY_F12, Value::F12},
        {KEY_PAUSE, Value::Pause},
        {KEY_INSERT, Value::Insert},
        {KEY_HOME, Value::Home},
        {KEY_PAGE_UP, Value::PageUp},
        {KEY_DELETE, Value::Delete},
        {KEY_END, Value::End},
        {KEY_PAGE_DOWN, Value::PageDown},
        {KEY_RIGHT, Value::Right},
        {KEY_LEFT, Value::Left},
        {KEY_DOWN, Value::Down},
        {KEY_UP, Value::Up},
        {KEY_SLASH, Value::Divide},
        {KEY_KP_DIVIDE, Value::Divide},
        {KEY_KP_MULTIPLY, Value::Multiply},
        {KEY_MINUS, Value::Subtract},
        {KEY_KP_SUBTRACT, Value::Subtract},
        {KEY_KP_ADD, Value::Add},
        {KEY_KP_1, Value::Numpad1},
        {KEY_KP_2, Value::Numpad2},
        {KEY_KP_3, Value::Numpad3},
        {KEY_KP_4, Value::Numpad4},
        {KEY_KP_5, Value::Numpad5},
        {KEY_KP_6, Value::Numpad6},
        {KEY_KP_7, Value::Numpad7},
        {KEY_KP_8, Value::Numpad8},
        {KEY_KP_9, Value::Numpad9},
        {KEY_KP_0, Value::Numpad0},
        {KEY_PERIOD, Value::Period},
        {KEY_LEFT_SHIFT, Value::LShift},
        {KEY_RIGHT_SHIFT, Value::RShift},
    };

    const std::unordered_map<Key::Layout, const Key::KeyMap &> Key::_layout = {
        {Layout::AZERTY, Key::_keyMapAzerty},
        {Layout::QWERTY, Key::_keyMapQwerty},
    };
}; // namespace RType
