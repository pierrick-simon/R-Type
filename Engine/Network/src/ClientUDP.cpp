/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ClientUDP.cpp
*/

#include "Network.hpp"

namespace Engine {

UDPClient::UDPClient(asio::io_context& ioContext, const std::string& host, short port) :
    context_(ioContext), socket_(ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)), 
    serverEndpoint_(asio::ip::address::from_string(host), port) 
{
    receive();
}

UDPClient::~UDPClient()
{
}

void UDPClient::login()
{
    send(Opcode::LOGIN, {});
}

void UDPClient::send(Opcode op, const std::vector<uint8_t>& msg)
{
    asio::post(context_, [this, op, msg]()
    {
        std::vector<uint8_t> payload(msg.begin(), msg.end());
        sendBuffer_ = serialize(Header{op, static_cast<uint16_t>(payload.size()), seq_++, myId_}, payload);
        socket_.async_send_to(asio::buffer(sendBuffer_), serverEndpoint_,
            [this](std::error_code, std::size_t) {});
    });
}


void UDPClient::receive()
{
    socket_.async_receive_from(asio::buffer(recvBuffer_), serverEndpoint_, [this](std::error_code ec, std::size_t bytesRecvd)
    {
        if (!ec && bytesRecvd > 0) {
            if (auto h = parse(recvBuffer_, bytesRecvd)) {
                manageCommands(h.value(), std::vector<uint8_t>(recvBuffer_.begin() + Header::SIZE, recvBuffer_.begin() + bytesRecvd));
            }
        }
        receive();
    });
}

void UDPClient::manageCommands(Header head, std::vector<uint8_t> payload)
{
    std::string text(payload.begin(), payload.end());
    //std::cout << "Réponse: " << text << std::endl;

    if (head.type == Opcode::LOGIN) {
        uint16_t p = (payload[0] << 8) | payload[1];
        myId_ = p;
    }
    return;
}

}
