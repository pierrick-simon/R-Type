/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Main
*/

#include <iostream>
#include <unistd.h>
#include "Clock.hpp"
#include "Friction2D.hpp"
#include "GameManager.hpp"
#include "KeyboardManager.hpp"
#include "Position2D.hpp"
#include "ServerManager.hpp"
#include "SetClientEntity.hpp"
#include "UpdateClientEntity.hpp"

RType::Entity createServer(RType::GameManager &gm)
{
    auto s = gm.createEntity();
    gm.addComponent<RType::ServerManager>(s, 12345);
    gm.createSystem<RType::ServerManager>(
        RType::ServerManager::systemServerReceived);
    return s;
}

RType::Entity createPlayer(RType::GameManager &gm)
{
    RType::Entity ship = gm.createEntity();
    gm.addComponent<RType::Position2D>(ship, 0.f, 0.f);
    gm.addComponent<RType::Velocity2D>(ship, 0.5, 0.5);
    gm.addComponent<RType::UpdateClientEntity>(
        ship, RType::EntityType::SPRITE, 0);

    return ship;
}

RType::Entity createText(RType::GameManager &gm, RType::Position2D pos)
{
    RType::Entity ship = gm.createEntity();
    gm.addComponent<RType::Position2D>(ship, pos.x(), pos.y());
    gm.addComponent<RType::UpdateClientEntity>(
        ship, RType::EntityType::TEXT_ENTITY, 1);
    gm.addComponent<RType::Text2D>(ship);

    return ship;
}

RType::Entity createMasterClock(RType::GameManager &gm)
{
    RType::Entity mc = gm.createEntity();
    gm.addComponent<RType::MasterClock>(mc);
    gm.createSystem<RType::MasterClock>(RType::MasterClock::systemMasterClock);
    gm.createSystem<RType::MasterClock>(
        RType::Velocity2D::systemMasterVelocity2D);
    return mc;
}

int main()
{
    try {
        RType::GameManager gm;
        std::unordered_map<int, std::string> map = {
            {0, "../loading.png"}, {1, "../font.ttf"}};
        gm.setAssetsMap(map);
        auto s = createServer(gm);
        auto p = createPlayer(gm);
        auto cm = createMasterClock(gm);
        auto t = createText(gm, {100, 100});
        auto t1 = createText(gm, {200, 200});

        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemUpdateEntityPos);
        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemCreateClientEntity);
        gm.createSystem<RType::ServerManager>(
            RType::UpdateClientEntity::systemUpdateTextEntity);
        gm.createSystem<RType::MasterClock>(
            RType::UpdateClientEntity::systemMasterUpdateClientEntitySleep);
        gm.createSystem<RType::ServerManager>(
            RType::SetClientEntity::systemUpdatesAllEntity<RType::Text2D,
                RType::Sprite2D,
                RType::SpriteSheet2D>);

        gm.createSystem<RType::Clock>(RType::Clock::systemClock);

        auto &sever = gm.getComponent<RType::ServerManager>(s);
        while (sever.isRunning()) {
            gm.update();
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
