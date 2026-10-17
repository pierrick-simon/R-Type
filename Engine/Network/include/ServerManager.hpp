/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ServerManager
*/

#ifndef SERVERMANAGER_HPP_
#define SERVERMANAGER_HPP_

#include "GameManager.hpp"
#include "ServerUDP.hpp"

namespace RType {

    /**
     * @class ServerManager
     * @brief ECS component that owns the UDP server and exposes it to the
     * engine.
     *
     * Wraps a UDPServer behind a std::unique_ptr so that Server itself is
     * movable and can be stored in a component array, while the UDPServer stays
     * at a fixed heap address (its Asio handlers capture `this` and must never
     * be relocated).
     *
     * @note Server is move-only. A moved-from Server holds a null UDPServer and
     *       must not be used.
     */
    class ServerManager {
    public:
        /**
         * @brief Creates the UDP server and starts listening.
         *
         * Binds a UDPServer to the given port and calls its start() method,
         * which begins the asynchronous receive loop and the periodic health
         * check. No thread is created: all network work runs on the caller's
         * thread inside systemServerReceived().
         *
         * @param port UDP port to bind to.
         * @param defaultProtocole apply the default protocole for the client
         * @throws asio::system_error If the socket cannot be bound to the port.
         */
        ServerManager(short port, bool defaultProtocole = true);

        /**
         * @brief ECS system that processes pending network events.
         *
         * Runs the Asio event loop without blocking, then handles every packet
         * received since the last call. The GameManager is provided so that
         * packet handlers can create, delete or modify entities.
         *
         * @param server Server component to update.
         * @param gm     Game manager giving handlers access to entities and
         *               components. It is only used during this call.
         *
         */
        static void systemServerReceived(
            ServerManager &server, GameManager &gm);

    private:
        /**
         * @brief Add the default manage function to the udp Server.
         */
        void createDefaultProtocol();

        void addContextManage(uint8_t op,
            const std::function<void(const Header &,
                const std::vector<uint8_t> &, std::shared_ptr<Context>)> &func);

        std::unique_ptr<UDPServer> _udp;
        std::shared_ptr<Context> _ctx = std::make_shared<Context>();
    };
} // namespace RType

#endif /* !SERVERMANAGER_HPP_ */
