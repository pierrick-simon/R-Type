/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolKey
*/

#include "ProtocolKey.hpp"

namespace RType {

    inline uint8_t toByte(Key::Value key)
    {
        return static_cast<uint8_t>(key);
    }

    inline std::optional<Key::Value> fromByte(uint8_t b)
    {
        if (b >= static_cast<uint8_t>(Key::Value::KeyCount))
            return std::nullopt;
        return static_cast<Key::Value>(b);
    }


    void ProtocolKey::manageKeyPressed(const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() < 1) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
    }


    void ProtocolKey::manageKeyRelease(const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.size() < 1) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
    }


    void ProtocolKey::systemKey(ClientManager &c, KeyboardManager &keys)
    {
        auto &pressed = keys.keyPressed;
        auto &released = keys.keyReleased;

        for (int i = 0; i < pressed.size(); i++)
            c.send(Opcode::KEY_PRESSED, {toByte(pressed[i])});
        pressed.clear();
        for (int i = 0; i < released.size(); i++)
            c.send(Opcode::KEY_RELEASED, {toByte(released[i])});
        released.clear();
    }

} //namespace RType
