/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Setup
*/

#include "Setup.hpp"
#include "Background.hpp"
#include "Bullet.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "Enemies/EnemySystems.hpp"
#include "Friction2D.hpp"
#include "KeyboardManager.hpp"
#include "Player.hpp"
#include "ServerManager.hpp"
#include "SetClientEntity.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"
#include "UpdateClientEntity.hpp"
#include "UpdatePosition2D.hpp"
#include "UpdateSprite2D.hpp"
#include "UpdateSpriteSheet2D.hpp"
#include "UpdateText2D.hpp"
#include "Velocity2D.hpp"
#include "Window.hpp"

namespace RType::Server {

    Entity createServer(GameManager &gm)
    {
        auto server = gm.createEntity();
        gm.addComponent<ServerManager>(server, 12345);
        gm.createSystem<ServerManager>(ServerManager::systemServerReceived);
        gm.createSystem<UpdateText2D, Text2D>(UpdateText2D::systemUpdateText2D);
        gm.createSystem<UpdateSprite2D, Sprite2D>(
            UpdateSprite2D::systemUpdateSprite2D);
        gm.createSystem<UpdateSpriteSheet2D, SpriteSheet2D>(
            UpdateSpriteSheet2D::systemUpdateSpriteSheet2D);
        gm.createSystem<UpdatePosition2D, Position2D>(
            UpdatePosition2D::systemUpdatePosition2D);
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
                UpdateSpriteSheet2D>);
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
        gm.createSystem<MasterClock>(
            SpriteSheet2D::systemMasterAnimateSpriteSheet2D);
        registerEnemySystems(gm, enemyHooks);
        gm.createSystem<>(systemCreateSpaceShip);
        gm.createSystem<MasterClock>(systemMasterPlayerHandle);
        gm.createSystem<MasterClock>([](GameManager &g, MasterClock &c) {
            for (auto [_, f, v] : g.view<Friction2D, Velocity2D>().getVector())
                Friction2D::systemFriction2D(f, v, c);
        });
        gm.createSystem<Background, Position2D>(backgroundLoop);
        gm.createSystem<Player, Position2D, Velocity2D>(playerClamp);
        gm.createSystem<Player, Velocity2D, SpriteSheet2D>(playerAnimation);
        gm.createSystem<MasterClock>(playerDeath);
        gm.createSystem<>(bulletSystem);
    }

} // namespace RType::Server
