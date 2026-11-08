/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterLink
*/

#ifndef MASTERLINK_HPP_
#define MASTERLINK_HPP_

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "Connexion/TcpConnection.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    inline constexpr std::chrono::seconds EMPTY_LOBBY_TIMEOUT {30};

    struct LinkedInstance {
        explicit LinkedInstance(TcpConnection link) :
            connection(std::move(link)), since(std::chrono::steady_clock::now())
        {
        }

        TcpConnection connection;
        uint16_t port = 0;
        int players = 0;
        bool started = false;
        bool everHadPlayers = false;
        bool stopSent = false;
        std::chrono::steady_clock::time_point since;
    };

    struct MasterLink {
        MasterLink();

        std::unique_ptr<asio::io_context> io;
        std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
        std::vector<LinkedInstance> instances;
    };

    void systemMasterLink(GameManager &gm);

} // namespace RType::Server

#endif /* !MASTERLINK_HPP_ */
