/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef CLIENTUDP_HPP
    #define CLIENTUDP_HPP

    #include "Network.hpp"

namespace RType {

    class UDPClient {
    public:
        UDPClient(const std::string& host, short port);
        ~UDPClient();
        void start();
        void stop();

        void send(Opcode op, const std::vector<uint8_t>& msg);
        bool isConnected() const;
        uint16_t getMyId() const;

    private:
        void receive();
        void manageCommands(const Header& head, const std::vector<uint8_t>& payload);

        void scheduleLogin();
        void sendNow(Opcode op, const std::vector<uint8_t>& msg);

        asio::io_context context_;
        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint serverEndpoint_;
        asio::ip::udp::endpoint senderEndpoint_; 
        std::vector<uint8_t> recvBuffer_ = std::vector<uint8_t>(1024);
        std::vector<uint8_t> sendBuffer_;
        asio::steady_timer loginTimer_;

        uint32_t seq_ = 0;
        std::atomic<uint16_t> myId_{0};
        std::thread thread_;
    };
    
}

#endif
