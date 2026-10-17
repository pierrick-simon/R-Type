/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** ClientUDP.cpp
*/

#include "ClientUDP.hpp"
#include "Position2D.hpp"
#include "Sprite2D.hpp"

namespace RType {

    UDPClient::UDPClient(const std::string &host, short port) :
        socket_(context_, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
        loginTimer_(context_),
        serverEndpoint_(asio::ip::make_address(host), port)
    {
    }

    UDPClient::~UDPClient()
    {
        stop();
    }

    void UDPClient::start()
    {
        receive();
        asio::post(context_, [this]() { scheduleLogin(); });
    }

    void UDPClient::stop()
    {
        context_.stop();
    }

    void UDPClient::received(GameManager &gm)
    {
        gm_ = std::ref(gm);
        if (context_.stopped())
            context_.restart();
        context_.poll();
        gm_.reset();
    }

    void UDPClient::scheduleLogin()
    {
        if (myId_ != 0)
            return;
        sendNow(Opcode::LOGIN, {});
        loginTimer_.expires_after(std::chrono::seconds(LOGIN_CHECK));
        loginTimer_.async_wait([this](std::error_code ec) {
            if (!ec)
                scheduleLogin();
        });
    }

    void UDPClient::send(Opcode op, const std::vector<uint8_t> &msg)
    {
        asio::post(context_, [this, op, msg]() { sendNow(op, msg); });
    }

    void UDPClient::sendNow(Opcode op, const std::vector<uint8_t> &msg)
    {
        sendBuffer_ = serialize(Header {op, 0, seq_++, myId_}, msg);
        socket_.async_send_to(asio::buffer(sendBuffer_),
            serverEndpoint_,
            [](std::error_code, std::size_t) {});
    }

    void UDPClient::receive()
    {
        socket_.async_receive_from(asio::buffer(recvBuffer_),
            senderEndpoint_,
            [this](std::error_code ec, std::size_t bytesRecvd) {
                if (!ec && bytesRecvd > 0) {
                    if (auto h = parse(recvBuffer_, bytesRecvd)) {
                        manageCommands(h.value(),
                            std::vector<uint8_t>(
                                recvBuffer_.begin() + Header::SIZE,
                                recvBuffer_.begin() + long(bytesRecvd)));
                    }
                }
                receive();
            });
    }

    void UDPClient::manageCommands(
        const Header &head, const std::vector<uint8_t> &payload)
    {
        for (const auto &[opcode, func] : protocolClient_) {
            if (head.type == opcode) {
                func(head, payload);
                return;
            }
        }
    }

    void UDPClient::manageLogin(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (payload.size() > 2)
            return;
        myId_ = (payload[0] << 8) | payload[1];
        std::cout << "My id = " << myId_ << std::endl;
    }

    void UDPClient::manageHealth(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        std::cout << "Health check from server" << std::endl;
        sendNow(Opcode::HEALTH, {});
    }

    uint16_t UDPClient::getMyId() const
    {
        return myId_;
    }

    bool UDPClient::isConnected() const
    {
        if (myId_ != 0)
            return true;
        return false;
    }

    void UDPClient::manageSprite(
        const Header &header, const std::vector<uint8_t> &payload)
    {
        if (!gm_)
            return;
        auto e = gm_->get().createEntity();
        gm_->get().addComponent<RType::Position2D>(e, 0, 0);
        gm_->get().addComponent<RType::Texture2D>(e, "loading.png");
        gm_->get().addComponent<RType::Sprite2D>(e);
    }

} // namespace RType
