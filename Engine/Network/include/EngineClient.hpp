/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EngineClient
*/

#ifndef ENGINECLIENT_HPP_
#define ENGINECLIENT_HPP_

#include "ClientUDP.hpp"

namespace RType {

    /**
     * @class EngineClient
     * @brief ECS component that owns the UDP client and exposes it to the
     * engine.
     *
     * Wraps a UDPClient behind a std::unique_ptr so that Client itself is
     * movable and can be stored in a component array, while the UDPClient stays
     * at a fixed heap address (its Asio handlers capture `this` and must never
     * be relocated).
     *
     * @note Client is move-only. A moved-from Client holds a null UDPClient and
     *       must not be used.
     */
    class EngineClient {
    public:
        /**
         * @brief Creates the UDP client targeting the given server.
         *
         * Builds a UDPClient configured for the given host and port. No thread
         * is created: all network work runs on the caller's thread inside
         * systemClientReceived().
         *
         * @param host Hostname or IP address of the server.
         * @param port UDP port of the server.
         * @throws asio::system_error If the host cannot be resolved or the
         * socket cannot be opened.
         */
        EngineClient(const std::string &host, short port);

        /**
         * @brief ECS system that processes pending network events.
         *
         * Runs the Asio event loop without blocking, then handles every packet
         * received from the server since the last call. The GameManager is
         * provided so that packet handlers can create, delete or modify
         * entities (for example to mirror the server's game state).
         *
         * @param client Client component to update.
         * @param gm     Game manager giving handlers access to entities and
         *               components. It is only used during this call.
         *
         */
        static void systemClientReceived(EngineClient &client, GameManager &gm);

    private:
        /// Owned UDP client, kept on the heap so its address never changes.
        std::unique_ptr<UDPClient> _udp;
    };
} // namespace RType

#endif /* !ENGINECLIENT_HPP_ */
