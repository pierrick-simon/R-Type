/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Instance
*/

#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include <cstdint>
#include <optional>
#include <thread>
#include <unordered_map>

namespace RType::Server {

    inline constexpr uint16_t FIRST_INSTANCE_PORT = 4243;
    inline constexpr uint16_t LAST_INSTANCE_PORT = 6767;

    struct InstanceManager {
        InstanceManager() = default;
        InstanceManager(InstanceManager &&) = default;
        InstanceManager &operator=(InstanceManager &&) = default;
        InstanceManager(const InstanceManager &) = delete;
        InstanceManager &operator=(const InstanceManager &) = delete;
        ~InstanceManager();

        std::unordered_map<uint16_t, std::thread> threads;
        std::unordered_map<uint32_t, uint16_t> portByClient;
        uint16_t nextPort = FIRST_INSTANCE_PORT;
    };

    std::optional<uint16_t> startInstance(InstanceManager &instances);

    void reapInstance(InstanceManager &instances, uint16_t port);

} // namespace RType::Server

#endif /* !INSTANCE_HPP_ */
