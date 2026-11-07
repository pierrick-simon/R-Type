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
#include "KeyboardManager.hpp"

namespace RType {

    /** @brief Game actions the player can bind to a key. */
    enum class Action : std::size_t { Up, Left, Down, Right, Shoot, Count };

    inline constexpr std::size_t ACTION_COUNT =
        static_cast<std::size_t>(Action::Count);

    /** @brief Key of each action expected by the server (ZQSD + Space). */
    inline constexpr std::array<Key::Value, ACTION_COUNT> DEFAULT_KEYS = {
        Key::Value::Z, Key::Value::Q, Key::Value::S, Key::Value::D,
        Key::Value::Space};

    /**
     * @brief Key chosen by the player for each action (client side).
     *
     * The server only knows the default keys: the client translates the
     * chosen key into the default one just before sending it (see
     * KeyBindings::translate and KeyboardProtocol::systemKeyboardProtocol).
     */
    inline std::array<Key::Value, ACTION_COUNT> _keyBindings = DEFAULT_KEYS;

    class KeyBindings {
    public:
        /**
         * @brief Key to send to the server for a key of the player.
         *
         * - a key bound to an action -> the default key of that action;
         * - a default key that is no longer bound (Z after binding Up to U)
         *   -> nothing, otherwise it would still move the ship;
         * - any other key -> sent as it is.
         */
        static std::optional<Key::Value> translate(Key::Value key);

        /** @brief Binds @p key to @p action. If another action had this key,
         *         the two actions swap their keys. */
        static void bind(Action action, Key::Value key);

        /** @brief Name of a key, with letters, digits and spaces only (the
         *         font of the game has no other character). */
        static std::string keyName(Key::Value key);

        static std::string actionName(Action action);
    };

} // namespace RType

#endif /* !KEYBINDINGS_HPP_ */
