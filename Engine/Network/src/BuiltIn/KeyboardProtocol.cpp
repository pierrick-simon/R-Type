/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "KeyboardProtocol.hpp"

namespace RType {

    uint8_t KeyboardProtocol::toByte(Key::Value key)
    {
        return static_cast<uint8_t>(key);
    }

    std::optional<Key::Value> KeyboardProtocol::fromByte(uint8_t b)
    {
        if (b >= static_cast<uint8_t>(Key::Value::KeyCount))
            return std::nullopt;
        return static_cast<Key::Value>(b);
    }

    void KeyboardProtocol::manageKeyPressed(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty()) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
    }

    void KeyboardProtocol::manageKeyRelease(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty()) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
    }

    void KeyboardProtocol::systemKey(ClientManager &c, KeyboardManager &km)
    {
        auto &pressed = km.keyPressed;
        auto &released = km.keyReleased;

        for (auto &value : pressed)
            c.send(Opcode::KEY_PRESSED, {toByte(value)});
        pressed.clear();
        for (auto &value : released)
            c.send(Opcode::KEY_RELEASED, {toByte(value)});
        released.clear();
    }

} // namespace RType
