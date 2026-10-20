/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef SERVERUDP_HPP_
    #define SERVERUDP_HPP_

    #include "Exceptions.hpp"
    #include "Network.hpp"

namespace RType {

    struct Client {
        uint32_t seq = 0;
        uint16_t id = 0;
        asio::ip::udp::endpoint endpoint;
        std::vector<uint8_t> sendBuffer;
        std::vector<std::string> msgSentHistory;
        std::pair<Header, std::vector<uint8_t>> lastMsgReceived;
        std::chrono::steady_clock::time_point lastSeen =
            std::chrono::steady_clock::now();
    };

    class UDPServer {
    public:
        UDPServer(short port);
        ~UDPServer();
        void start();
        void stop();

        void received();

        void broadcast(Opcode op, const std::vector<uint8_t> &payload);
        void sendToClient(
            Client &client, Opcode op, const std::vector<uint8_t> &payload);
        void sendToClient(
            uint32_t &id, Opcode op, const std::vector<uint8_t> &payload);

        void addManage(uint8_t opCode, ManageFunc func, bool override = false);

        uint16_t addNewClient();
        void removeClient(const Header &header);

        class ServerException : public EngineException {
        public:
            ServerException(const std::string &str) :
                EngineException("Server error: " + str) {};
        };

    private:
        void startReceive();
        void manageCommand(
            const Header &header, const std::vector<uint8_t> &payload);
        void scheduleHealthCheck();
        void checkClients();
        void broadcastNow(Opcode op, const std::vector<uint8_t> &payload);

        asio::io_context _context;
        asio::ip::udp::socket _socket;
        asio::steady_timer _healthTimer;

        asio::ip::udp::endpoint _remoteEndpoint;
        std::vector<uint8_t> _recvBuffer = std::vector<uint8_t>(1024);
        std::vector<uint8_t> _sendBuffer;
        std::map<uint16_t, Client> _clients;
        uint16_t _nextId = 0;
        uint16_t _serverId = 0;

        void manageLogin(
            const Header &header, const std::vector<uint8_t> &payload);
        static void manageHealth(
            const Header &header, const std::vector<uint8_t> &payload);

        std::map<uint8_t,
            std::function<void(const Header &, const std::vector<uint8_t> &)>>
            _protocol = {
                {Opcode::LOGIN,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        manageLogin(h, p);
                    }},
                {Opcode::LOGOUT,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        removeClient(h);
                    }},
                {Opcode::HEALTH,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        manageHealth(h, p);
                    }},
        };
    };

} // namespace RType

#endif
