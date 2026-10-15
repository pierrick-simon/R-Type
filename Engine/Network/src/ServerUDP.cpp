/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ServerUDP.cpp
*/

#include <asio.hpp>
#include <iostream>
#include <vector>
#include <string>

#include "Network.hpp"

namespace Engine {

UDPServer::UDPServer(short port) :
    socket_(context_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), healthTimer_(context_)
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
    thread_ = std::thread([this]()
        {
            context_.run(); 
        });
}

void UDPServer::stop()
{
    context_.stop();
    if (thread_.joinable())
        thread_.join();
}

void UDPServer::scheduleHealthCheck()
{
    healthTimer_.expires_after(std::chrono::seconds(HEALTH_CHECK_TIME));
    healthTimer_.async_wait([this](std::error_code ec)
        {
            if (ec)
                return;
            checkClients();
            scheduleHealthCheck();
        });
}

void UDPServer::checkClients()
{
    auto now = std::chrono::steady_clock::now();

    for (auto it = clients_.begin(); it != clients_.end(); ) {
        if (now - it->second.lastSeen > std::chrono::seconds(TIMEOUT)) {
            std::cout << "Timeout from client nb " << it->second.id << " ..." << std::endl;
            it = clients_.erase(it);
        } else
            ++it;
    }
    broadcastNow(Opcode::HEALTH, {});
    std::cout << "Health check from serv" << std::endl;
}

void UDPServer::startReceive()
{
    socket_.async_receive_from(asio::buffer(recvBuffer_), remoteEndpoint_,
        [this](std::error_code ec, std::size_t bytesRecvd)
        {
            if (!ec && bytesRecvd > 0) {
                if (auto header = parse(recvBuffer_, bytesRecvd)) {
                    manageCommand(header.value(), std::vector<uint8_t>(recvBuffer_.begin() + Header::SIZE, recvBuffer_.begin() + bytesRecvd));
                }
            }
            startReceive();
        });
}

void UDPServer::sendToClient(Client& client, Opcode op, const std::vector<uint8_t>& payload)
{
    client.sendBuffer_ = serialize(Header{op, static_cast<uint16_t>(payload.size()), client.seq++, serverId_}, payload);
    socket_.async_send_to(asio::buffer(client.sendBuffer_), client.endpoint, [this](std::error_code, std::size_t) {});
}

void UDPServer::broadcast(Opcode op, const std::vector<uint8_t>& payload)
{
    asio::post(context_, [this, op, payload]()
        {
        for (auto& [id, client] : clients_)
            sendToClient(client, op, payload);
        });
}

void UDPServer::broadcastNow(Opcode op, const std::vector<uint8_t>& payload)
{
    for (auto& [id, client] : clients_)
        sendToClient(client, op, payload);
}

void UDPServer::manageCommand(Header header, std::vector<uint8_t> payload)
{
    if (header.type == Opcode::LOGIN) {
        login(header, payload);
        return;
    }
    auto it = clients_.find(header.senderId);
    if (it == clients_.end() || it->second.endpoint != remoteEndpoint_)
        return;
    std::string text(payload.begin(), payload.end());
    std::cout << "Reçu de ." << header.senderId << ". : " << text << std::endl;
    it->second.lastSeen = std::chrono::steady_clock::now();
    if (header.type == Opcode::HEALTH)
        return; 
    return;
}

void UDPServer::login(Header header, std::vector<uint8_t> payload)
{
    std::cout << "Login" << std::endl;
    nbClients += 1;
    Client& c = clients_[nbClients];

    c.id = nbClients;
    c.endpoint = remoteEndpoint_;
    std::cout << "nb clients :" << nbClients << std::endl;
    sendToClient(c, Opcode::LOGIN, {static_cast<uint8_t>(nbClients >> 8), static_cast<uint8_t>(nbClients)});
}


}
