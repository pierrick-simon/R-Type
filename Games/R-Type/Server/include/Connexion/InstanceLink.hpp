/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** InstanceLink
*/

#ifndef INSTANCELINK_HPP_
#define INSTANCELINK_HPP_

#include <cstdint>
#include <memory>

#include "Connexion/TcpConnection.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    inline constexpr uint16_t MASTER_LINK_PORT = 4240;

    enum LinkMessageType : uint8_t {
        LINK_HELLO = 1,
        LINK_STATUS = 2,
        LINK_STOP = 3,
        LINK_SCORE = 4,
    };

    struct InstanceLink {
        explicit InstanceLink(uint16_t instancePort);

        std::unique_ptr<asio::io_context> io;
        TcpConnection connection;
        int lastPlayers = -1;
        bool lastStarted = false;
        bool stopRequested = false;

    private:
        static asio::ip::tcp::socket connectToMaster(asio::io_context &io);
    };

    void systemInstanceLink(GameManager &gm);

    void reportScores(GameManager &gm);

} // namespace RType::Server

#endif /* !INSTANCELINK_HPP_ */
