/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TcpConnection
*/
#include <array>

#include "TcpConnection.hpp"

namespace RType::Server {

    TcpConnection::TcpConnection(asio::ip::tcp::socket socket) :
        _socket(std::move(socket))
    {
        std::error_code ec;

        _socket.non_blocking(true, ec);
        _socket.set_option(asio::ip::tcp::no_delay(true), ec);
    }

    void TcpConnection::send(uint8_t type, const std::vector<uint8_t> &payload)
    {
        const std::size_t at = _out.size();

        _out.resize(at + HEADER_SIZE + payload.size());
        _out[at] = type;
        put16(&_out[at + 1], static_cast<uint16_t>(payload.size()));
        std::copy(payload.begin(), payload.end(), _out.begin() + long(at + 3));
    }

    void TcpConnection::poll()
    {
        std::array<uint8_t, 512> buffer {};
        std::error_code ec;

        if (_closed)
            return;
        while (!_out.empty()) {
            const std::size_t n = _socket.write_some(asio::buffer(_out), ec);
            if (ec == asio::error::would_block)
                break;
            if (ec) {
                _closed = true;
                return;
            }
            _out.erase(_out.begin(), _out.begin() + long(n));
        }
        while (true) {
            const std::size_t n = _socket.read_some(asio::buffer(buffer), ec);
            if (ec == asio::error::would_block)
                break;
            if (ec) {
                _closed = true;
                return;
            }
            _in.insert(_in.end(), buffer.begin(), buffer.begin() + long(n));
        }
    }

    std::optional<LinkMessage> TcpConnection::nextMessage()
    {
        if (_in.size() < HEADER_SIZE)
            return std::nullopt;
        const std::size_t size = get16(&_in[1]);
        if (_in.size() < HEADER_SIZE + size)
            return std::nullopt;
        LinkMessage message;
        message.type = _in[0];
        message.payload.assign(_in.begin() + long(HEADER_SIZE),
            _in.begin() + long(HEADER_SIZE + size));
        _in.erase(_in.begin(), _in.begin() + long(HEADER_SIZE + size));
        return message;
    }

} // namespace RType::Server
