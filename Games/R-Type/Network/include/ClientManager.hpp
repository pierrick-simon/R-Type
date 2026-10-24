/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ClientManager
*/

#ifndef CLIENTMANAGER_HPP_
#define CLIENTMANAGER_HPP_

#include "ClientUDP.hpp"
#include "Context.hpp"
#include "GameManager.hpp"

namespace RType {

    /**
     * @class ClientManager
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
    class ClientManager {
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
        ClientManager(const std::string &host, short port);

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
        static void systemClientReceived(
            GameManager &gm, ClientManager &client);

        static void systemEntityCreated(ClientManager &c);

        void send(uint8_t op, const std::vector<uint8_t> &payload);

        void addCreatedEntity(uint32_t id);

        [[nodiscard]] Context &getContext()
        {
            return _ctx;
        }

    private:
        /**
         * @brief Add the manage function to the udp Client.
         */
        void createProtocol();

        void addContextManage(uint8_t op,
            const std::function<void(const Header &,
                const std::vector<uint8_t> &, Context &)> &func);

        std::unique_ptr<UDPClient> _udp;
        Context _ctx;
        std::vector<uint32_t> _created;
        bool _first = true;
    };
} // namespace RType

#endif /* !CLIENTMANAGER_HPP_ */
