/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network.hpp
*/

#ifndef SERVERUDP_HPP_
    #define SERVERUDP_HPP_

    #include "GameManager.hpp"
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

        void received(GameManager &gm);

        void broadcast(Opcode op, const std::vector<uint8_t> &payload);
        void sendToClient(
            Client &client, Opcode op, const std::vector<uint8_t> &payload);

    private:
        void startReceive();
        void manageCommand(
            const Header &header, const std::vector<uint8_t> &payload);
        void scheduleHealthCheck();
        void checkClients();
        void broadcastNow(Opcode op, const std::vector<uint8_t> &payload);

        asio::io_context context_;
        asio::ip::udp::socket socket_;
        asio::steady_timer healthTimer_;

        asio::ip::udp::endpoint remoteEndpoint_;
        std::vector<uint8_t> recvBuffer_ = std::vector<uint8_t>(1024);
        std::vector<uint8_t> sendBuffer_;
        std::map<uint16_t, Client> clients_;
        uint16_t nextId_ = 0;
        uint16_t serverId_ = 0;

        std::optional<std::reference_wrapper<GameManager>> gm_;

        void manageLogin(
            const Header &header, const std::vector<uint8_t> &payload);
        static void manageHealth(
            const Header &header, const std::vector<uint8_t> &payload);
        void manageLogout(
            const Header &header, const std::vector<uint8_t> &payload);

        std::map<Opcode,
            std::function<void(const Header &, const std::vector<uint8_t> &)>>
            protocol_ = {
                {Opcode::LOGIN,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        manageLogin(h, p);
                    }},
                {Opcode::LOGOUT,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        manageLogout(h, p);
                    }},
                {Opcode::HEALTH,
                    [this](const Header &h, const std::vector<uint8_t> &p) {
                        manageHealth(h, p);
                    }},
        };
    };

} // namespace RType

#endif
