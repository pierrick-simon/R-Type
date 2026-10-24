/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Context
*/

#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_

    #include "GameManager.hpp"
    #include "Network.hpp"

namespace RType {

    enum OpcodeRType : uint8_t {
        KEY_PRESSED = 10,
        KEY_RELEASED = 11,
        MOUSE_BUTTON_PRESSED = 12,
        MOUSE_BUTTON_RELEASED = 13,
        CREATE = 20,
        MOVE = 21,
        SET = 22,
        TEXT = 23,
        PLAY = 24,
        PAUSE = 25,
        STOP = 26,
        DESTROY = 27,
        MOVE_VELOCITY = 28
    };

    struct Context {
        std::unordered_map<uint16_t, Entity> clients;
        std::optional<std::reference_wrapper<GameManager>> gm;
        std::unordered_map<size_t, Entity> map;
    };
}; // namespace RType

#endif /* !CONTEXT_HPP_ */
