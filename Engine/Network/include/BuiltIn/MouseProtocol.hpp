/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MouseProtocol
*/

#ifndef MOUSEPROTOCOL_HPP_
    #define MOUSEPROTOCOL_HPP_

#include "ClientManager.hpp"
#include "GameManager.hpp"
#include "MouseManager.hpp"
#include "Network.hpp"

namespace RType {

    class MouseProtocol {
    public:
        static void manageButtonPressed(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void manageButtonReleased(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void systemMouseProtocol(GameManager &gm, ClientManager &c);

    private:
        static std::optional<MouseButton::Value> fromByte(uint8_t b);
        static uint8_t toByte(MouseButton::Value key);
    };

} // namespace RType

#endif
