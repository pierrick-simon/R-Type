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

    enum Opcode : uint8_t { LOGIN = 1, LOGOUT, HEALTH };

    struct Header {
        static constexpr std::size_t SIZE = 1 + 2 + 4 + 2;
        uint8_t type;
        uint16_t payloadSize;
        uint32_t sequence;
        uint16_t senderId;
    };

    using ManageFunc =
        std::function<void(const Header &, const std::vector<uint8_t> &)>;

    std::optional<Header> parse(
        const std::vector<uint8_t> &buf, std::size_t len);
    std::vector<uint8_t> serialize(
        Header h, const std::vector<uint8_t> &payload);

    struct Context {
        std::optional<std::reference_wrapper<GameManager>> gm;
        std::unordered_map<size_t, Entity> map;
    };

} // namespace RType

#endif
