/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef NETWORK_HPP_
    #define NETWORK_HPP_

    #include <map>
    #include <string>
    #include <iostream>
    #include <asio.hpp>
    #include <cstdint>
    #include <optional>

namespace Engine {

    enum class Opcode : uint8_t {
        LOGIN = 1,
        LOGOUT,
        HEALTH
    };

    struct Header {
        static constexpr std::size_t SIZE = 1 + 2 + 4 + 2;
        Opcode type;
        uint16_t payloadSize;
        uint32_t sequence;
        uint16_t senderId;
    };


    struct Client {
        uint32_t seq = 0;
        uint16_t id = 0;
        asio::ip::udp::endpoint endpoint;
        std::vector<uint8_t> sendBuffer_;
    };


    class UDPServer {
        public:
            UDPServer(asio::io_context& io_context, short port);
            ~UDPServer();
            void broadcast(Opcode op, const std::vector<uint8_t>& payload);
            void sendToClient(Client &client, Opcode op, const std::vector<uint8_t>& payload);

        private:
            void startReceive();
            void manageCommand(Header header, std::vector<uint8_t> payload);
            void login(Header header, std::vector<uint8_t> payload);

            asio::io_context& context_;
            asio::ip::udp::socket socket_;
            asio::ip::udp::endpoint remoteEndpoint_;
            std::vector<uint8_t> recvBuffer_ = std::vector<uint8_t>(1024);
            std::vector<uint8_t> sendBuffer_;
            std::map<uint16_t, Client> clients_;
            uint16_t nbClients = 0;
            uint16_t serverId_ = 0;
    };


    class UDPClient {
    public:
        UDPClient(asio::io_context& io_context, const std::string& host, short port);
        ~UDPClient();
        void send(Opcode op, const std::vector<uint8_t>& msg);
        void login();

    private:
        void receive();
        void manageCommands(Header head, std::vector<uint8_t> payload);
        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint serverEndpoint_;
        std::vector<uint8_t> recvBuffer_ = std::vector<uint8_t>(1024);
        std::vector<uint8_t> sendBuffer_;
        uint32_t seq_ = 0;
        uint16_t myId_ = 0;
        asio::io_context& context_;
    };
    
    std::optional<Header> parse(const std::vector<uint8_t>& buf, std::size_t len);
    std::vector<uint8_t> serialize(Header h, const std::vector<uint8_t>& payload);
    
}

#endif
