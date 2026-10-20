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

        void send(Opcode op, const std::vector<uint8_t> &msg);
        [[nodiscard]] bool isConnected() const;
        [[nodiscard]] uint16_t getMyId() const;

        void addManage(Opcode op, ManageFunc func, bool override = false);

        class ClientException : public EngineException {
        public:
            ClientException(const std::string &str) :
                EngineException("Client error: " + str) {};
        };

    private:
        void receive();
        void manageCommands(
            const Header &head, const std::vector<uint8_t> &payload);

        void scheduleLogin();
        void sendNow(Opcode op, const std::vector<uint8_t> &msg);

        asio::io_context _context;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _serverEndpoint;
        asio::ip::udp::endpoint _senderEndpoint;
        std::vector<uint8_t> _recvBuffer = std::vector<uint8_t>(1024);
        std::vector<uint8_t> _sendBuffer;
        asio::steady_timer _loginTimer;

        uint32_t _seq = 0;
        std::atomic<uint16_t> _myId {0};

        void manageLogin(
            const Header &header, const std::vector<uint8_t> &payload);
        void manageHealth(
            const Header &header, const std::vector<uint8_t> &payload);

        std::map<uint8_t, ManageFunc> _protocolClient = {
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
