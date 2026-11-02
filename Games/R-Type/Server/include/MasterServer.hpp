/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MasterServer
*/

#ifndef MASTERSERVER_HPP_
#define MASTERSERVER_HPP_

#include "GameManager.hpp"

namespace RType::Server {

    /**
     * @brief Creates the master server (port LOGIN_SERVER_PORT): no game,
     * it only starts lobby instances on demand and sends their port.
     * @return The entity holding the ServerManager and the InstanceManager.
     */
    Entity createMasterServer(GameManager &gm);

    /**
     * @brief Handles CREATE_LOBBY requests: starts one instance per client
     * (or re-sends the same port if the request was repeated) and answers
     * with LOBBY_PORT.
     */
    void systemMasterRequests(GameManager &gm);

} // namespace RType::Server

#endif /* !MASTERSERVER_HPP_ */
