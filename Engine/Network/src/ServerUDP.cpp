/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ServerUDP.cpp
*/

#include "ServerUDP.hpp"

namespace RType {

    UDPServer::UDPServer(short port) :
        _socket(_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
        _healthTimer(_context)
    {
    }

    UDPServer::~UDPServer()
    {
        stop();
    }

    void UDPServer::start()
    {
        startReceive();
        scheduleHealthCheck();
    }

    void UDPServer::stop()
    {
        _context.stop();
    }

    void UDPServer::received()
    {
        if (_context.stopped())
            _context.restart();
        _context.poll();
    }

    void UDPServer::scheduleHealthCheck()
    {
        _healthTimer.expires_after(std::chrono::seconds(HEALTH_CHECK_TIME));
        _healthTimer.async_wait([this](std::error_code ec) {
            if (ec)
                return;
            checkClients();
            scheduleHealthCheck();
        });
    }

    void UDPServer::checkClients()
    {
        auto now = std::chrono::steady_clock::now();

        for (auto it = _clients.begin(); it != _clients.end();) {
            if (now - it->second.lastSeen > std::chrono::seconds(TIMEOUT)) {
                std::cout << "Timeout from client nb " << it->second.id
                          << " ..." << std::endl;
                it = _clients.erase(it);
            } else
                ++it;
        }
        broadcastNow(Opcode::HEALTH, {});
        std::cout << "Health check from serv" << std::endl;
    }

    void UDPServer::startReceive()
    {
        _socket.async_receive_from(asio::buffer(_recvBuffer),
            _remoteEndpoint,
            [this](std::error_code ec, std::size_t bytesRecvd) {
                if (!ec && bytesRecvd > 0) {
                    if (auto header = parseHeader(_recvBuffer, bytesRecvd)) {
                        manageCommand(header.value(),
                            std::vector<uint8_t>(
                                _recvBuffer.begin() + Header::SIZE,
                                _recvBuffer.begin() + long(bytesRecvd)));
                    }
                }
                startReceive();
            });
    }

    void UDPServer::sendToClient(
        Client &client, Opcode op, const std::vector<uint8_t> &payload)
    {
        client.sendBuffer =
            serializeHeader(Header {op,
                                static_cast<uint16_t>(payload.size()),
                                client.seq++,
                                _serverId},
                payload);
        _socket.async_send_to(asio::buffer(client.sendBuffer),
            client.endpoint,
            [this](std::error_code, std::size_t) {});
        client.msgSentHistory.push_back("[" +
            std::to_string(static_cast<int>(op)) + "] " +
            std::string(payload.begin(), payload.end()));
    }

    void UDPServer::sendToClient(
        uint32_t &id, Opcode op, const std::vector<uint8_t> &payload)
    {
        if (!_clients.count(id))
            throw(
                ServerException("Client " + std::to_string(id) + " not found"));
        sendToClient(_clients.at(id), op, payload);
    }

    void UDPServer::broadcast(Opcode op, const std::vector<uint8_t> &payload)
    {
        asio::post(_context, [this, op, payload]() {
            for (auto &[id, client] : _clients)
                sendToClient(client, op, payload);
        });
    }

    void UDPServer::broadcastNow(Opcode op, const std::vector<uint8_t> &payload)
    {
        for (auto &[id, client] : _clients)
            sendToClient(client, op, payload);
    }

    void UDPServer::manageCommand(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (header.type == Opcode::LOGIN) {
            auto login = _protocol.find(Opcode::LOGIN);
            if (login != _protocol.end())
                login->second(header, payload);
            return;
        }
        auto it = _clients.find(header.senderId);
        if (it == _clients.end() || it->second.endpoint != _remoteEndpoint)
            return;
        it->second.lastSeen = std::chrono::steady_clock::now();
        it->second.lastMsgReceived = {header, payload};
        auto handler = _protocol.find(header.type);
        if (handler != _protocol.end())
            handler->second(header, payload);
    }

    void UDPServer::addManage(uint8_t op, ManageFunc func, bool override)
    {
        if (!override && _protocol.count(op))
            throw EngineException("Opcode already use for a manage function");
        _protocol[op] = std::move(func);
    }

    void UDPServer::removeClient(const Header &header)
    {
        std::cout << "Client " << header.senderId << " logged out" << std::endl;
        _clients.erase(header.senderId);
    }

    void UDPServer::manageHealth(const Header &, const std::vector<uint8_t> &)
    {
    }

    void UDPServer::manageLogin(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        _nextId += 1;
        Client &c = _clients[_nextId];

        c.id = _nextId;
        c.endpoint = _remoteEndpoint;
        sendToClient(c,
            Opcode::LOGIN,
            {static_cast<uint8_t>(_nextId >> 8),
                static_cast<uint8_t>(_nextId)});
    }

    uint16_t UDPServer::addNewClient()
    {
        _nextId += 1;
        Client &c = _clients[_nextId];

        c.id = _nextId;
        c.endpoint = _remoteEndpoint;
        sendToClient(c,
            Opcode::LOGIN,
            {static_cast<uint8_t>(_nextId >> 8),
                static_cast<uint8_t>(_nextId)});
        return _nextId;
    }

} // namespace RType
