/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ClientUDP.cpp
*/

#include "ClientUDP.hpp"

namespace RType {

    UDPClient::UDPClient(const std::string &host, short port) :
        _socket(_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
        _loginTimer(_context),
        _serverEndpoint(asio::ip::make_address(host), port)
    {
    }

    UDPClient::~UDPClient()
    {
        sendNow(Opcode::LOGOUT, {});
        stop();
    }

    void UDPClient::start()
    {
        receive();
        asio::post(_context, [this]() { scheduleLogin(); });
    }

    void UDPClient::stop()
    {
        _context.stop();
    }

    void UDPClient::received()
    {
        if (_context.stopped())
            _context.restart();
        _context.poll();
    }

    void UDPClient::scheduleLogin()
    {
        if (_myId != 0)
            return;
        sendNow(Opcode::LOGIN, {});
        _loginTimer.expires_after(std::chrono::seconds(LOGIN_CHECK));
        _loginTimer.async_wait([this](std::error_code ec) {
            if (!ec)
                scheduleLogin();
        });
    }

    void UDPClient::send(uint8_t op, const std::vector<uint8_t> &msg)
    {
        asio::post(_context, [this, op, msg]() { sendNow(op, msg); });
    }

    void UDPClient::sendNow(uint8_t op, const std::vector<uint8_t> &msg)
    {
        _sendBuffer = serializeHeader(Header {op, 0, _seq++, _myId}, msg);
        _socket.async_send_to(asio::buffer(_sendBuffer),
            _serverEndpoint,
            [](std::error_code, std::size_t) {});
    }

    void UDPClient::receive()
    {
        _socket.async_receive_from(asio::buffer(_recvBuffer),
            _senderEndpoint,
            [this](std::error_code ec, std::size_t bytesRecvd) {
                if (!ec && bytesRecvd > 0) {
                    if (auto h = parseHeader(_recvBuffer, bytesRecvd)) {
                        manageCommands(h.value(),
                            std::vector<uint8_t>(
                                _recvBuffer.begin() + Header::SIZE,
                                _recvBuffer.begin() + long(bytesRecvd)));
                    }
                }
                receive();
            });
    }

    void UDPClient::addManage(uint8_t op, ManageFunc func, bool override)
    {
        if (!override && _protocolClient.count(op))
            throw EngineException("Opcode already use for a manage function");
        _protocolClient[op] = std::move(func);
    }

    void UDPClient::manageCommands(
        const Header &head, const std::vector<uint8_t> &payload)
    {
        for (const auto &[opcode, func] : _protocolClient) {
            if (head.type == opcode) {
                func(head, payload);
                return;
            }
        }
    }

    void UDPClient::manageLogin(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (payload.size() < 2)
            return;
        _myId = (payload[0] << 8) | payload[1];
        std::cout << "My id = " << _myId << std::endl;
    }

    void UDPClient::manageHealth(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        std::cout << "Health check from server" << std::endl;
        sendNow(Opcode::HEALTH, {});
    }

    uint16_t UDPClient::getMyId() const
    {
        return _myId;
    }

    bool UDPClient::isConnected() const
    {
        if (_myId != 0)
            return true;
        return false;
    }

} // namespace RType
