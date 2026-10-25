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
#include "Keyboard.hpp"
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

    // Entity createWindow(GameManager &gm)
    // {
    //     auto window = gm.createEntity();

    //     gm.addComponent<Window>(window, WINDOW_WIDTH, WINDOW_HEIGHT,
    //     "R-type"); gm.createSystem<Window>(Window::systemClearWindow); return
    //     window;
    // }

    // Entity createKeyboardManager(GameManager &gm)
    // {
    //     auto keyboardManager = gm.createEntity();

    //     gm.addComponent<KeyboardManager>(keyboardManager);
    //     gm.createSystem<KeyboardManager>(
    //         KeyboardManager::systemKeyboardManager);
    //     return keyboardManager;
    // }

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
        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemUpdateEntityPos);
        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemCreateClientEntity);
        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemUpdateTextEntity);
        gm.createSystem<RType::MasterClock>(
            RType::UpdateClientEntity::systemMasterUpdateClientEntitySleep);
        gm.createSystem<RType::ServerManager>(
            RType::SetClientEntity::systemUpdatesAllEntity<RType::UpdateText2D,
                RType::UpdateSprite2D,
                RType::UpdateSpriteSheet2D>);
        return server;
    }

    RType::Entity createMasterClock(RType::GameManager &gm)
    {
        RType::Entity mc = gm.createEntity();
        gm.addComponent<RType::MasterClock>(mc);
        gm.createSystem<RType::MasterClock>(
            RType::MasterClock::systemMasterClock);
        gm.createSystem<RType::MasterClock>(
            RType::Velocity2D::systemMasterVelocity2D);
        return mc;
    }

    void createSystems(GameManager &gm, const EnemyHooks &enemyHooks)
    {
        gm.createSystem<Clock>(Clock::systemClock);
        registerEnemySystems(gm, enemyHooks);
        // gm.createSystem<Player,
        //     Velocity2D,
        //     Position2D,
        //     Keyboard,
        //     Clock,
        //     Health>(playerHandle);
        gm.createSystem<Friction2D, Velocity2D, Clock>(
            Friction2D::systemFriction2D);
        // gm.createSystem<Velocity2D, Position2D, Clock>(
        //     Velocity2D::systemVelocity2D);
        gm.createSystem<Background, Position2D>(backgroundLoop);
        gm.createSystem<Player, Position2D, Velocity2D>(playerClamp);
        gm.createSystem<Player, Velocity2D, SpriteSheet2D>(playerAnimation);
        gm.createSystem<>(playerDeath);
        gm.createSystem<SpriteSheet2D, Clock>(
            SpriteSheet2D::systemAnimateSpriteSheet2D);
        gm.createSystem<>(bulletSystem);
    }

} // namespace RType::Server
