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

UDPServer::UDPServer(asio::io_context& ioContext, short port) :
    context_(ioContext), socket_(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
{
    startReceive();
}

UDPServer::~UDPServer()
{
}

void UDPServer::startReceive()
{
    socket_.async_receive_from(asio::buffer(recvBuffer_), remoteEndpoint_,
        [this](std::error_code ec, std::size_t bytesRecvd) {
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
    return;
}

void UDPServer::login(Header header, std::vector<uint8_t> payload)
{
    std::cout << "Login" << std::endl;
    nbClients += 1;
    Client& c = clients_[nbClients];

    c.id = nbClients;
    c.endpoint = remoteEndpoint_;
    //std::cout << "nb clients :" << nbClients << std::endl;
    sendToClient(c, Opcode::LOGIN, {static_cast<uint8_t>(nbClients >> 8), static_cast<uint8_t>(nbClients)});
}


}
