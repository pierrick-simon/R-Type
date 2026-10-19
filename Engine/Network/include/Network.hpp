/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef NETWORK_HPP_
    #define NETWORK_HPP_

    #include <asio.hpp>
    #include <atomic>
    #include <chrono>
    #include <cstdint>
    #include <functional>
    #include <iostream>
    #include <map>
    #include <optional>
    #include <string>
    #include "GameManager.hpp"

namespace RType {

    #define HEALTH_CHECK_TIME 2
    #define TIMEOUT 6
    #define LOGIN_CHECK 3

    enum Opcode : uint8_t {
        LOGIN = 1,
        LOGOUT = 2,
        HEALTH = 3,
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
        DESTROY = 27
    };

    struct Header {
        static constexpr std::size_t SIZE = 1 + 2 + 4 + 2;
        uint8_t type;
        uint16_t payloadSize;
        uint32_t sequence;
        uint16_t senderId;
    };

    using ManageFunc =
        std::function<void(const Header &, const std::vector<uint8_t> &)>;

    std::optional<Header> parseHeader(
        const std::vector<uint8_t> &buf, std::size_t len);
    std::vector<uint8_t> serializeHeader(
        Header h, const std::vector<uint8_t> &payload);

    uint32_t get32(const uint8_t *p);
    uint16_t get16(const uint8_t *p);
    void put32(uint8_t *p, uint32_t v);
    void put16(uint8_t *p, uint16_t v);

    struct Context {
        std::unordered_map<uint16_t, Entity> clients;
        std::optional<std::reference_wrapper<GameManager>> gm;
        std::unordered_map<size_t, Entity> map;
    };

} // namespace RType

#endif
