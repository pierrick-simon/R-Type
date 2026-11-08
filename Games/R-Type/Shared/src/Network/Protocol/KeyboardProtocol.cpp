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
        if (!ctx.gm || payload.empty() || !ctx.clients.count(header.senderId)) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
        auto &keyboard = ctx.gm->get().getComponent<KeyboardManager>(
            ctx.clients.at(header.senderId));
        keyboard.keyPressed.emplace_back(key.value());
        if (std::find(keyboard.keyDown.begin(),
                keyboard.keyDown.end(),
                key.value()) == keyboard.keyDown.end())
            keyboard.keyDown.emplace_back(key.value());
    }

    void KeyboardProtocol::manageKeyReleased(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty() || !ctx.clients.count(header.senderId)) {
            return;
        }
        auto key = fromByte(payload[0]);
        if (!key)
            return;
        auto &keyboard = ctx.gm->get().getComponent<KeyboardManager>(
            ctx.clients.at(header.senderId));
        keyboard.keyReleased.emplace_back(key.value());
        keyboard.keyDown.erase(
            std::remove(
                keyboard.keyDown.begin(), keyboard.keyDown.end(), key.value()),
            keyboard.keyDown.end());
    }

    void KeyboardProtocol::systemKeyboardProtocol(
        GameManager &gm, ClientManager &c)
    {
        gm.view<KeyboardManager>().each([&c](Entity e, KeyboardManager &km) {
            auto &pressed = km.keyPressed;
            auto &released = km.keyReleased;
            // Keys are already remapped by the engine Keybind (see
            // KeyBindings::fillKeybind), so they are sent as they are.
            auto send = [&c](uint8_t op, Key::Value value) {
                c.send(op, {toByte(value)});
            };
            for (auto &value : pressed)
                send(OpcodeRType::KEY_PRESSED, value);
            for (auto &value : released)
                send(OpcodeRType::KEY_RELEASED, value);
            for (auto &value : pressed)
                if (std::ranges::find(km.keyDown, value) == km.keyDown.end() &&
                    std::ranges::find(released, value) == released.end())
                    send(OpcodeRType::KEY_RELEASED, value);
        });
    }

} // namespace RType
