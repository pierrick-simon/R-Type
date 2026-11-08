/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** InputBinding
*/

#include "InputBinding.hpp"
#include "GameManagerBinding.hpp"
#include "KeyboardManager.hpp"
#include "MouseManager.hpp"

namespace RType::Game {

    void InputBinding::registerFunctions(sol::state &lua)
    {
        lua.new_enum<Key::Value>("Key",
            {{"A", Key::Value::A},
                {"B", Key::Value::B},
                {"C", Key::Value::C},
                {"D", Key::Value::D},
                {"E", Key::Value::E},
                {"F", Key::Value::F},
                {"G", Key::Value::G},
                {"H", Key::Value::H},
                {"I", Key::Value::I},
                {"J", Key::Value::J},
                {"K", Key::Value::K},
                {"L", Key::Value::L},
                {"M", Key::Value::M},
                {"N", Key::Value::N},
                {"O", Key::Value::O},
                {"P", Key::Value::P},
                {"Q", Key::Value::Q},
                {"R", Key::Value::R},
                {"S", Key::Value::S},
                {"T", Key::Value::T},
                {"U", Key::Value::U},
                {"V", Key::Value::V},
                {"W", Key::Value::W},
                {"X", Key::Value::X},
                {"Y", Key::Value::Y},
                {"Z", Key::Value::Z},
                {"Num0", Key::Value::Num0},
                {"Num1", Key::Value::Num1},
                {"Num2", Key::Value::Num2},
                {"Num3", Key::Value::Num3},
                {"Num4", Key::Value::Num4},
                {"Num5", Key::Value::Num5},
                {"Num6", Key::Value::Num6},
                {"Num7", Key::Value::Num7},
                {"Num8", Key::Value::Num8},
                {"Num9", Key::Value::Num9},
                {"Escape", Key::Value::Escape},
                {"LControl", Key::Value::LControl},
                {"LShift", Key::Value::LShift},
                {"LAlt", Key::Value::LAlt},
                {"LSystem", Key::Value::LSystem},
                {"RControl", Key::Value::RControl},
                {"RShift", Key::Value::RShift},
                {"RAlt", Key::Value::RAlt},
                {"RSystem", Key::Value::RSystem},
                {"Menu", Key::Value::Menu},
                {"LBracket", Key::Value::LBracket},
                {"RBracket", Key::Value::RBracket},
                {"Semicolon", Key::Value::Semicolon},
                {"Comma", Key::Value::Comma},
                {"Period", Key::Value::Period},
                {"Apostrophe", Key::Value::Apostrophe},
                {"Slash", Key::Value::Slash},
                {"Backslash", Key::Value::Backslash},
                {"Grave", Key::Value::Grave},
                {"Equal", Key::Value::Equal},
                {"Hyphen", Key::Value::Hyphen},
                {"Space", Key::Value::Space},
                {"Enter", Key::Value::Enter},
                {"Backspace", Key::Value::Backspace},
                {"Tab", Key::Value::Tab},
                {"PageUp", Key::Value::PageUp},
                {"PageDown", Key::Value::PageDown},
                {"End", Key::Value::End},
                {"Home", Key::Value::Home},
                {"Insert", Key::Value::Insert},
                {"Delete", Key::Value::Delete},
                {"Add", Key::Value::Add},
                {"Subtract", Key::Value::Subtract},
                {"Multiply", Key::Value::Multiply},
                {"Divide", Key::Value::Divide},
                {"Left", Key::Value::Left},
                {"Right", Key::Value::Right},
                {"Up", Key::Value::Up},
                {"Down", Key::Value::Down},
                {"Numpad0", Key::Value::Numpad0},
                {"Numpad1", Key::Value::Numpad1},
                {"Numpad2", Key::Value::Numpad2},
                {"Numpad3", Key::Value::Numpad3},
                {"Numpad4", Key::Value::Numpad4},
                {"Numpad5", Key::Value::Numpad5},
                {"Numpad6", Key::Value::Numpad6},
                {"Numpad7", Key::Value::Numpad7},
                {"Numpad8", Key::Value::Numpad8},
                {"Numpad9", Key::Value::Numpad9},
                {"F1", Key::Value::F1},
                {"F2", Key::Value::F2},
                {"F3", Key::Value::F3},
                {"F4", Key::Value::F4},
                {"F5", Key::Value::F5},
                {"F6", Key::Value::F6},
                {"F7", Key::Value::F7},
                {"F8", Key::Value::F8},
                {"F9", Key::Value::F9},
                {"F10", Key::Value::F10},
                {"F11", Key::Value::F11},
                {"F12", Key::Value::F12},
                {"F13", Key::Value::F13},
                {"F14", Key::Value::F14},
                {"F15", Key::Value::F15},
                {"Pause", Key::Value::Pause}});
        lua.new_usertype<KeyboardManager>("KeyboardManager",
            sol::no_constructor,
            "isPressed",
            contains(&KeyboardManager::keyPressed),
            "isDown",
            contains(&KeyboardManager::keyDown),
            "isReleased",
            contains(&KeyboardManager::keyReleased));
        lua["GameManager"]["keyboard"] =
            [](GameManager &gm) -> KeyboardManager & {
            return GameManagerBinding::findModuleComponent<KeyboardManager>(
                gm, "gm:keyboard", "Keyboard");
        };

        lua.new_enum<MouseButton::Value>("MouseButton",
            {
                {"Left", MouseButton::Value::Left},
                {"Right", MouseButton::Value::Right},
                {"Middle", MouseButton::Value::Middle},
                {"Side", MouseButton::Value::Side},
                {"Extra", MouseButton::Value::Extra},
                {"Forward", MouseButton::Value::Forward},
                {"Back", MouseButton::Value::Back},
            });
        lua.new_usertype<MouseManager>("MouseManager",
            sol::no_constructor,
            "isPressed",
            contains(&MouseManager::buttonPressed),
            "isDown",
            contains(&MouseManager::buttonDown),
            "isReleased",
            contains(&MouseManager::buttonReleased),
            "x",
            sol::readonly_property(
                [](const MouseManager &m) { return m.getPos().x; }),
            "y",
            sol::readonly_property(
                [](const MouseManager &m) { return m.getPos().y; }),
            "wheel",
            sol::readonly_property(&MouseManager::getWheel)),
            "deltaX", sol::readonly_property([](const MouseManager &m) {
                return m.getDelta().x;
            }),
            "deltaY", sol::readonly_property([](const MouseManager &m) {
                return m.getDelta().y;
            });
        lua["GameManager"]["mouse"] = [](GameManager &gm) -> MouseManager & {
            return GameManagerBinding::findModuleComponent<MouseManager>(
                gm, "gm:mouse", "Mouse");
        };
    }

} // namespace RType::Game
