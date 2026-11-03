/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include "Animation.hpp"
#include "Background.hpp"
#include "Bullet.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemySystems.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "Friction2D.hpp"
#include "KeyboardManager.hpp"
#include "Lobby.hpp"
#include "Player.hpp"
#include "ServerManager.hpp"
#include "SetClientEntity.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"
#include "UpdatePosition2D.hpp"
#include "UpdateSound.hpp"
#include "UpdateSprite2D.hpp"
#include "UpdateSpriteSheet2D.hpp"
#include "UpdateText2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace RType::Server {

    Entity createServer(GameManager &gm, short port)
    {
        return createServer(gm, ServerManager(port));
    }

    Entity createServer(GameManager &gm, ServerManager &&manager)
    {
        auto server = gm.createEntity();
        gm.addComponent<ServerManager>(server, std::move(manager));
        gm.createSystem<ServerManager>(ServerManager::systemServerReceived);
        gm.createSystem<UpdateText2D, Text2D>(UpdateText2D::systemUpdateText2D);
        gm.createSystem<UpdateSprite2D, Sprite2D>(
            UpdateSprite2D::systemUpdateSprite2D);
        gm.createSystem<UpdateSpriteSheet2D, SpriteSheet2D>(
            UpdateSpriteSheet2D::systemUpdateSpriteSheet2D);
        gm.createSystem<UpdatePosition2D, Position2D>(
            UpdatePosition2D::systemUpdatePosition2D);
        gm.createSystem<UpdateSound, SoundInfo>(UpdateSound::systemUpdateSound);
        gm.createSystem<ServerManager>(
            UpdateClientEntity::systemUpdateEntityPos);
        gm.createSystem<ServerManager>(
            UpdateClientEntity::systemCreateClientEntity);
        gm.createSystem<ServerManager>(
            UpdateClientEntity::systemUpdateTextEntity);
        gm.createSystem<MasterClock>(
            UpdateClientEntity::systemMasterUpdateClientEntitySleep);
        gm.createSystem<ServerManager>(
            SetClientEntity::systemUpdatesAllEntity<UpdateText2D,
                UpdateSprite2D,
                UpdateSpriteSheet2D,
                UpdateSound>);
        gm.createSystem<ServerManager>([&gm](ServerManager &s) {
            for (auto [_, u] : gm.view<UpdateClientEntity>().getVector())
                for (auto [id, _] : s.getContext().clients)
                    u.create.push_back(id);
        });
        return server;
    }

    RType::Entity createMasterClock(GameManager &gm)
    {
        Entity mc = gm.createEntity();
        gm.addComponent<MasterClock>(mc);
        gm.createSystem<MasterClock>(MasterClock::systemMasterClock);
        gm.createSystem<MasterClock>(Velocity2D::systemMasterVelocity2D);
        return mc;
    }

    void createSystems(GameManager &gm, const EnemyHooks &enemyHooks)
    {
        gm.createSystem<>(systemLobby);
        gm.createSystem<MasterClock>(
            SpriteSheet2D::systemMasterAnimateSpriteSheet2D);
        gm.createSystem<PlayOnce, SpriteSheet2D>(playOnceAnimation);
        registerEnemySystems(gm, enemyHooks);
        registerEnemyVisualSystems(gm);
        gm.createSystem<>(systemCreateSpaceShip);
        gm.createSystem<MasterClock>(systemMasterPlayerHandle);
        gm.createSystem<MasterClock>(Friction2D::systemMasterFriction2D);
        gm.createSystem<Background, Position2D>(backgroundLoop);
        gm.createSystem<Player, Position2D, Velocity2D>(playerClamp);
        gm.createSystem<Player, Velocity2D, SpriteSheet2D>(playerAnimation);
        gm.createSystem<>(playerTerrainCollision);
        gm.createSystem<MasterClock>(playerDeath);
        gm.createSystem<>(bulletSystem);
    }

} // namespace RType::Server
