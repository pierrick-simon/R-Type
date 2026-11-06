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

#include "Common/Background.hpp"
#include "Components.hpp"
#include "Connexion/Lobby.hpp"
#include "Enemies/EnemyManager.hpp"
#include "GameState.hpp"
#include "KeyboardManager.hpp"
#include "LobbyProtocol.hpp"
#include "MouseManager.hpp"
#include "Player/Destroy.hpp"
#include "Player/Hud.hpp"
#include "ServerManager.hpp"

namespace RType::Server {

    namespace {

        thread_local bool gameStarted = false;
        thread_local std::chrono::steady_clock::time_point lastBroadcast {};
        thread_local std::vector<uint8_t> lastPayload;
        thread_local std::string lobbyName;

        template<typename Component>
        void destroyAllOf(GameManager &gm)
        {
            std::vector<Entity> entities;

            gm.view<Component>().each(
                [&entities](Entity e, Component &) { entities.push_back(e); });
            for (const Entity e : entities)
                destroyNetworked(gm, e);
        }

        LobbyState buildState(GameManager &gm, ServerManager &server)
        {
            LobbyState state;
            std::vector<std::pair<uint16_t, Entity>> clients(
                server.getClients().begin(), server.getClients().end());

            std::sort(clients.begin(),
                clients.end(),
                [](const auto &a, const auto &b) { return a.first < b.first; });
            state.started = gameStarted;
            state.gameOver = isGameOverScreen();
            state.lobbyName = lobbyName;
            state.hostId = clients.empty() ? 0 : clients.front().first;
            for (const auto &[id, entity] : clients) {
                LobbyPlayerInfo info;

                info.id = id;
                if (gm.hasComponent<LobbyMember>(entity)) {
                    const auto &member = gm.getComponent<LobbyMember>(entity);
                    info.name = member.name;
                    info.ready = member.ready;
                    info.replay = member.replayVote;
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
            spawnTeamScore(gm);
            // The ships of a previous game wait, dead, in the lobby.
            resetPlayers(gm);
            for (auto [entity, player] : gm.view<Player>().getVector())
                spawnPlayerScore(gm, entity, player);
        }

        void resetLevel(GameManager &gm)
        {
            gameStarted = false;
            closeGameOver();
            resetEnemies(gm);
            // Only gone at the end of the frame: stops the game state first.
            for (auto [_, director] : gm.view<EnemyDirector>().getVector())
                director.paused = true;
            destroyAllOf<EnemyDirector>(gm);
            destroyAllOf<Background>(gm);
            destroyAllOf<Bullet>(gm);
            destroyAllOf<Force>(gm);
            destroyAllOf<ChargeEffect>(gm);
            destroyAllOf<ScoreLabel>(gm);
            destroyAllOf<PlayerExplosion>(gm);
        }

        void handleRequests(
            GameManager &gm, ServerManager &server, const LobbyState &state)
        {
            for (const auto &[id, entity] : server.getClients()) {
                if (!gm.hasComponent<LobbyMember>(entity))
                    continue;
                auto &member = gm.getComponent<LobbyMember>(entity);
                const bool host = id == state.hostId;
                const bool start = member.startRequested;
                const bool restart = member.restartRequested;
                const bool back = member.returnRequested;

                member.startRequested = false;
                member.restartRequested = false;
                member.returnRequested = false;
                if (start && host && !gameStarted && state.allReady())
                    startGame(gm);
                member.replayVote =
                    isGameOverScreen() && (member.replayVote || restart);
                if (host && back && isGameOverScreen()) {
                    std::cout << "[Lobby] The host goes back to the lobby"
                              << std::endl;
                    resetLevel(gm);
                }
            }
        }

        /** @brief Restarts once every connected player asked to play again. */
        void handleReplayVotes(GameManager &gm, ServerManager &server)
        {
            std::size_t votes = 0;

            if (!isGameOverScreen() || server.getClients().empty())
                return;
            for (const auto &[id, entity] : server.getClients())
                if (gm.hasComponent<LobbyMember>(entity) &&
                    gm.getComponent<LobbyMember>(entity).replayVote)
                    ++votes;
            if (votes < server.getClients().size())
                return;
            std::cout << "[Lobby] Every player wants to play again"
                      << std::endl;
            restartGame(gm);
            for (const auto &[id, entity] : server.getClients())
                if (gm.hasComponent<LobbyMember>(entity))
                    gm.getComponent<LobbyMember>(entity).replayVote = false;
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

            if (payload == lastPayload &&
                now - lastBroadcast < LOBBY_BROADCAST_RATE)
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

    void setLobbyName(const std::string &name)
    {
        lobbyName = name;
    }

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
        if (gameStarted && server->getClients().empty()) {
            std::cout << "[Lobby] Every player left, back to the lobby"
                      << std::endl;
            resetLevel(gm);
        }
        LobbyState state = buildState(gm, *server);

        handleRequests(gm, *server, state);
        handleReplayVotes(gm, *server);
        state = buildState(gm, *server);
        state.started = gameStarted;
        state.gameOver = isGameOverScreen();
        flushLobbyInputs(gm);
        broadcastState(*server, state);
    }

} // namespace RType::Server
