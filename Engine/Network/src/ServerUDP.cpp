/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ServerUDP.cpp
*/

#include "ServerUDP.hpp"

namespace RType {

    UDPServer::UDPServer(short port) :
        socket_(context_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
        healthTimer_(context_)
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
        context_.stop();
    }

    void UDPServer::received()
    {
        if (context_.stopped())
            context_.restart();
        context_.poll();
    }

    void UDPServer::scheduleHealthCheck()
    {
        healthTimer_.expires_after(std::chrono::seconds(HEALTH_CHECK_TIME));
        healthTimer_.async_wait([this](std::error_code ec) {
            if (ec)
                return;
            checkClients();
            scheduleHealthCheck();
        });
    }

    void UDPServer::checkClients()
    {
        auto now = std::chrono::steady_clock::now();

        for (auto it = clients_.begin(); it != clients_.end();) {
            if (now - it->second.lastSeen > std::chrono::seconds(TIMEOUT)) {
                std::cout << "Timeout from client nb " << it->second.id
                          << " ..." << std::endl;
                it = clients_.erase(it);
            } else
                ++it;
        }
        broadcastNow(Opcode::HEALTH, {});
        std::cout << "Health check from serv" << std::endl;
    }

    void UDPServer::startReceive()
    {
        socket_.async_receive_from(asio::buffer(recvBuffer_),
            remoteEndpoint_,
            [this](std::error_code ec, std::size_t bytesRecvd) {
                if (!ec && bytesRecvd > 0) {
                    if (auto header = parseHeader(recvBuffer_, bytesRecvd)) {
                        manageCommand(header.value(),
                            std::vector<uint8_t>(
                                recvBuffer_.begin() + Header::SIZE,
                                recvBuffer_.begin() + long(bytesRecvd)));
                    }
                }
                startReceive();
            });
    }

    void UDPServer::sendToClient(
        Client &client, Opcode op, const std::vector<uint8_t> &payload)
    {
        client.sendBuffer = serializeHeader(Header {op,
                                          static_cast<uint16_t>(payload.size()),
                                          client.seq++,
                                          serverId_},
            payload);
        socket_.async_send_to(asio::buffer(client.sendBuffer),
            client.endpoint,
            [this](std::error_code, std::size_t) {});
        client.msgSentHistory.push_back("[" +
            std::to_string(static_cast<int>(op)) + "] " +
            std::string(payload.begin(), payload.end()));
    }

    void UDPServer::broadcast(Opcode op, const std::vector<uint8_t> &payload)
    {
        asio::post(context_, [this, op, payload]() {
            for (auto &[id, client] : clients_)
                sendToClient(client, op, payload);
        });
    }

    void UDPServer::broadcastNow(
        Opcode op, const std::vector<uint8_t> &payload)
    {
        for (auto &[id, client] : clients_)
            sendToClient(client, op, payload);
    }

    void UDPServer::manageCommand(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (header.type == Opcode::LOGIN) {
            auto login = protocol_.find(Opcode::LOGIN);
            if (login != protocol_.end())
                login->second(header, payload);
            return;
        }
        auto it = clients_.find(header.senderId);
        if (it == clients_.end() || it->second.endpoint != remoteEndpoint_)
            return;
        it->second.lastSeen = std::chrono::steady_clock::now();
        it->second.lastMsgReceived = {header, payload};
        auto handler = protocol_.find(header.type);
        if (handler != protocol_.end())
            handler->second(header, payload);
    }

    void UDPServer::addManage(uint8_t op, ManageFunc func, bool override)
    {
        if (!override && protocol_.count(op))
            throw EngineException("Opcode already use for a manage function");
        protocol_[op] = std::move(func);
    }

    void UDPServer::manageLogout(
        const Header &header, const std::vector<uint8_t> &)
    {
        std::cout << "Client " << header.senderId << " logged out" << std::endl;
        clients_.erase(header.senderId);
    }

    void UDPServer::manageHealth(const Header &, const std::vector<uint8_t> &)
    {
    }

    void UDPServer::manageLogin(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        nextId_ += 1;
        Client &c = clients_[nextId_];

        c.id = nextId_;
        c.endpoint = remoteEndpoint_;
        sendToClient(c,
            Opcode::LOGIN,
            {static_cast<uint8_t>(nextId_ >> 8),
                static_cast<uint8_t>(nextId_)});
    }

    uint16_t UDPServer::addNewClient()
    {
        nextId_ += 1;
        Client &c = clients_[nextId_];

        c.id = nextId_;
        c.endpoint = remoteEndpoint_;
        sendToClient(c,
            Opcode::LOGIN,
            {static_cast<uint8_t>(nextId_ >> 8),
                static_cast<uint8_t>(nextId_)});
        return nextId_;
    }

} // namespace RType
