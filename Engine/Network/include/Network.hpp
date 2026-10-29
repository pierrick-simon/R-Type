/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef NETWORK_HPP_
    #define NETWORK_HPP_

    #if defined(_WIN32)
        #define WIN32_LEAN_AND_MEAN
        #define NOGDI
        #define NOUSER
    #endif
    #include <asio.hpp>
    #if defined(_WIN32)
        #undef near
        #undef far
    #endif
    #include <atomic>
    #include <chrono>
    #include <cstdint>
    #include <functional>
    #include <iostream>
    #include <map>
    #include <optional>
    #include <string>

namespace RType {

    #define HEALTH_CHECK_TIME 2
    #define TIMEOUT 10
    #define LOGIN_CHECK 2

    enum Opcode : uint8_t {
        LOGIN = 1,
        LOGOUT = 2,
        HEALTH = 3,
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

} // namespace RType

#endif
