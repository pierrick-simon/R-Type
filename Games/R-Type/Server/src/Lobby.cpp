/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lobby
*/

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "Lobby.hpp"
#include "Background.hpp"
#include "Components.hpp"
#include "Enemies/EnemyManager.hpp"
#include "KeyboardManager.hpp"
#include "LobbyProtocol.hpp"
#include "MouseManager.hpp"
#include "ServerManager.hpp"

namespace RType::Server {

    namespace {

        bool gameStarted = false;
        std::chrono::steady_clock::time_point lastBroadcast {};
        std::vector<uint8_t> lastPayload;

        template<typename Component>
        void destroyAllOf(GameManager &gm)
        {
            std::vector<Entity> entities;

            gm.view<Component>().each(
                [&entities](Entity e, Component &) { entities.push_back(e); });
            for (const Entity e : entities)
                gm.destroyEntity(e);
        }

        LobbyState buildState(GameManager &gm, ServerManager &server)
        {
            LobbyState state;
            std::vector<std::pair<uint16_t, Entity>> clients(
                server.getClients().begin(), server.getClients().end());

            std::sort(clients.begin(), clients.end(),
                [](const auto &a, const auto &b) { return a.first < b.first; });
            state.started = gameStarted;
            state.hostId = clients.empty() ? 0 : clients.front().first;
            for (const auto &[id, entity] : clients) {
                LobbyPlayerInfo info;

                info.id = id;
                if (gm.hasComponent<LobbyMember>(entity)) {
                    const auto &member = gm.getComponent<LobbyMember>(entity);
                    info.name = member.name;
                    info.ready = member.ready;
                }
                if (info.name.empty())
                    info.name = "Player " + std::to_string(id);
                state.players.push_back(std::move(info));
            }
            return state;
        }

        void startGame(GameManager &gm)
        {
            std::cout << "[Lobby] Game started by the host" << std::endl;
            gameStarted = true;
            createBackground(gm);
            createEnemyDirector(gm);
        }

        void resetLevel(GameManager &gm)
        {
            std::cout << "[Lobby] Every player left, back to the lobby"
                      << std::endl;
            gameStarted = false;
            resetEnemies(gm);
            destroyAllOf<EnemyDirector>(gm);
            destroyAllOf<Background>(gm);
            destroyAllOf<Bullet>(gm);
            destroyAllOf<PlayerExplosion>(gm);
        }

        void handleStartRequests(GameManager &gm, ServerManager &server,
            const LobbyState &state)
        {
            for (const auto &[id, entity] : server.getClients()) {
                if (!gm.hasComponent<LobbyMember>(entity))
                    continue;
                auto &member = gm.getComponent<LobbyMember>(entity);

                if (!member.startRequested)
                    continue;
                member.startRequested = false;
                if (!gameStarted && id == state.hostId && state.allReady())
                    startGame(gm);
            }
        }

        void flushLobbyInputs(GameManager &gm)
        {
            gm.view<DefaulClient, KeyboardManager>().each(
                [&gm](Entity e, DefaulClient &, KeyboardManager &k) {
                    if (gm.hasComponent<Player>(e))
                        return;
                    k.keyPressed.clear();
                    k.keyReleased.clear();
                    k.keyDown.clear();
                });
            gm.view<DefaulClient, MouseManager>().each(
                [&gm](Entity e, DefaulClient &, MouseManager &m) {
                    if (gm.hasComponent<Player>(e))
                        return;
                    m.buttonPressed.clear();
                    m.buttonReleased.clear();
                    m.buttonDown.clear();
                });
        }

        void broadcastState(ServerManager &server, const LobbyState &state)
        {
            const auto now = std::chrono::steady_clock::now();
            std::vector<uint8_t> payload = LobbyProtocol::serializeState(state);

            if (payload == lastPayload && now - lastBroadcast < LOBBY_BROADCAST_RATE)
                return;
            lastBroadcast = now;
            lastPayload = payload;
            try {
                server.broadcast(OpcodeRType::LOBBY_STATE, payload);
            } catch (const std::exception &e) {
                std::cerr << "[Lobby] " << e.what() << std::endl;
            }
        }

    } // namespace

    bool isGameStarted()
    {
        return gameStarted;
    }

    void systemLobby(GameManager &gm)
    {
        ServerManager *server = nullptr;

        gm.view<ServerManager>().each(
            [&server](Entity, ServerManager &s) { server = &s; });
        if (server == nullptr)
            return;
        if (gameStarted && server->getClients().empty())
            resetLevel(gm);
        LobbyState state = buildState(gm, *server);

        handleStartRequests(gm, *server, state);
        state.started = gameStarted;
        flushLobbyInputs(gm);
        broadcastState(*server, state);
    }

} // namespace RType::Server
