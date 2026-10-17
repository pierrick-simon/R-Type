/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** KeyboardProtocol
*/

#ifndef KEYBOARDPROTOCOL_HPP_
    #define KEYBOARDPROTOCOL_HPP_

#include "ClientManager.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Network.hpp"

namespace RType {

    class KeyboardProtocol {
    public:
        static void manageKeyPressed(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageKeyRelease(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void systemKey(ClientManager &c, KeyboardManager &km);

    private:
        static std::optional<Key::Value> fromByte(uint8_t b);
        static uint8_t toByte(Key::Value key);
    };

} // namespace RType

#endif
