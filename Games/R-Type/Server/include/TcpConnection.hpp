/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TcpConnection
*/

#ifndef TCPCONNECTION_HPP_
    #define TCPCONNECTION_HPP_

#include <cstdint>
#include <optional>
#include <vector>

#include "Network.hpp"

namespace RType::Server {

    struct LinkMessage {
        uint8_t type = 0;
        std::vector<uint8_t> payload;
    };

    class TcpConnection {
    public:
        explicit TcpConnection(asio::ip::tcp::socket socket);

        void send(uint8_t type, const std::vector<uint8_t> &payload = {});

        void poll();

        std::optional<LinkMessage> nextMessage();

        [[nodiscard]] bool isClosed() const
        {
            return _closed;
        }

    private:
        static constexpr std::size_t HEADER_SIZE = 3;

        asio::ip::tcp::socket _socket;
        std::vector<uint8_t> _in;
        std::vector<uint8_t> _out;
        bool _closed = false;
    };

} // namespace RType::Server

#endif /* !TCPCONNECTION_HPP_ */
