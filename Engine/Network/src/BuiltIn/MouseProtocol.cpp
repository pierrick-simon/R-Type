/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#include "MouseProtocol.hpp"

namespace RType {

    uint8_t MouseProtocol::toByte(MouseButton::Value key)
    {
        return static_cast<uint8_t>(key);
    }

    std::optional<MouseButton::Value> MouseProtocol::fromByte(uint8_t b)
    {
        if (b >= static_cast<uint8_t>(MouseButton::Value::MouseButtonCount))
            return std::nullopt;
        return static_cast<MouseButton::Value>(b);
    }

    void MouseProtocol::manageButtonPressed(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty() || !ctx.clients.count(header.senderId)) {
            return;
        }
        auto button = fromByte(payload[0]);
        if (!button)
            return;
        auto &mouse = ctx.gm->get().getComponent<MouseManager>(
            ctx.clients.at(header.senderId));
        mouse.buttonPressed.emplace_back(button.value());
        if (std::find(mouse.buttonDown.begin(),
                mouse.buttonDown.end(),
                button.value()) == mouse.buttonDown.end())
            mouse.buttonDown.emplace_back(button.value());
    }

    void MouseProtocol::manageButtonReleased(
        const Header &header, const std::vector<uint8_t> &payload, Context &ctx)
    {
        if (!ctx.gm || payload.empty() || !ctx.clients.count(header.senderId)) {
            return;
        }
        auto button = fromByte(payload[0]);
        if (!button)
            return;
        auto &mouse = ctx.gm->get().getComponent<MouseManager>(
            ctx.clients.at(header.senderId));
        mouse.buttonReleased.emplace_back(button.value());
        mouse.buttonDown.erase(std::remove(mouse.buttonDown.begin(),
                                   mouse.buttonDown.end(),
                                   button.value()),
            mouse.buttonDown.end());
    }

    void MouseProtocol::systemMouseProtocol(GameManager &gm, ClientManager &c)
    {
        gm.view<MouseManager>().each([&c](Entity e, MouseManager &mm) {
            auto &pressed = mm.buttonPressed;
            auto &released = mm.buttonReleased;
            for (auto &value : pressed)
                c.send(Opcode::MOUSE_BUTTON_PRESSED, {toByte(value)});
            for (auto &value : released)
                c.send(Opcode::MOUSE_BUTTON_RELEASED, {toByte(value)});
        });
    }

} // namespace RType
