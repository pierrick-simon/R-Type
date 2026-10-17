/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef CLIENTUDP_HPP
    #define CLIENTUDP_HPP

    #include "Exceptions.hpp"
    #include "Network.hpp"

namespace RType {

    class UDPClient {
    public:
        UDPClient(const std::string &host, short port);
        ~UDPClient();
        void start();
        void stop();

        void received();

        void send(uint8_t op, const std::vector<uint8_t> &msg);
        [[nodiscard]] bool isConnected() const;
        [[nodiscard]] uint16_t getMyId() const;

        void addManage(uint8_t op, ManageFunc func);

        class ClientException : public EngineException {
            ClientException(const std::string &str) :
                EngineException("Client error: " + str) {};
        };

    private:
        void receive();
        void manageCommands(
            const Header &head, const std::vector<uint8_t> &payload);

        void scheduleLogin();
        void sendNow(uint8_t op, const std::vector<uint8_t> &msg);

        asio::io_context context_;
        asio::ip::udp::socket socket_;
        asio::ip::udp::endpoint serverEndpoint_;
        asio::ip::udp::endpoint senderEndpoint_;
        std::vector<uint8_t> recvBuffer_ = std::vector<uint8_t>(1024);
        std::vector<uint8_t> sendBuffer_;
        asio::steady_timer loginTimer_;

        uint32_t seq_ = 0;
        std::atomic<uint16_t> myId_ {0};

        void manageLogin(
            const Header &header, const std::vector<uint8_t> &payload);
        void manageHealth(
            const Header &header, const std::vector<uint8_t> &payload);

        std::map<uint8_t, ManageFunc> protocolClient_ = {
            {Opcode::LOGIN,
                [this](const Header &h, const std::vector<uint8_t> &p) {
                    manageLogin(h, p);
                }},
            {Opcode::HEALTH,
                [this](const Header &h, const std::vector<uint8_t> &p) {
                    manageHealth(h, p);
                }},
        };
    };

} // namespace RType

#endif
