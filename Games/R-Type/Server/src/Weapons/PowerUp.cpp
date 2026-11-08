/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** PowerUp
*/

#include "Weapons/PowerUp.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "Assets.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Player/Destroy.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"
#include "Velocity2D.hpp"
#include "Weapons/Force.hpp"

namespace RType::Server {

    raylib::Color laserColor(LaserType laser)
    {
        switch (laser) {
            case LaserType::Helix:
                return {255, 110, 110, 255};
            case LaserType::Ring:
                return {110, 255, 170, 255};
            default:
                return {120, 160, 255, 255};
        }
    }

    bool touches(const Position2D &aPos, const Hitbox &aBox,
        const Position2D &bPos, const Hitbox &bBox)
    {
        const float ax = aPos.x + aBox.offsetX;
        const float ay = aPos.y + aBox.offsetY;
        const float bx = bPos.x + bBox.offsetX;
        const float by = bPos.y + bBox.offsetY;

        return ax < bx + bBox.width && bx < ax + aBox.width &&
            ay < by + bBox.height && by < ay + aBox.height;
    }

    Entity spawnPowerUp(GameManager &gm, LaserType laser, float x, float y)
    {
        const Entity item = gm.createEntity();
        SpriteSheet2D sheet(
            Size2D(1, POWER_UP_FRAMES), POWER_UP_FRAME_MS, SCALE);

        sheet.color = laserColor(laser);
        gm.addComponent<PowerUp>(item, PowerUp {laser});
        gm.addComponent<Position2D>(
            item, x - POWER_UP_WIDTH / 2.f, y - POWER_UP_HEIGHT / 2.f);
        gm.addComponent<Velocity2D>(item, -BACKGROUND_SPEED, 0.f);
        gm.addComponent<Hitbox>(item, Hitbox {POWER_UP_WIDTH, POWER_UP_HEIGHT});
        gm.addComponent<Lifetime>(item, Lifetime {POWER_UP_LIFETIME});
        gm.addComponent<SpriteSheet2D>(item, sheet);
        gm.addComponent<UpdateClientEntity>(
            item, EntityType::SPRITESHEET, Asset::POWER_UP, gm, item);
        return item;
    }

    void dropPowerUp(GameManager &gm, float x, float y)
    {
        thread_local std::mt19937 rng {std::random_device {}()};
        std::uniform_real_distribution<float> roll(0.f, 1.f);
        std::uniform_int_distribution<int> type(0, int(LaserType::Count) - 1);

        if (roll(rng) < POWER_UP_DROP_CHANCE)
            spawnPowerUp(gm, LaserType(type(rng)), x, y);
    }

    void powerUpPickupSystem(GameManager &gm)
    {
        std::vector<Entity> served;

        for (auto [item, powerUp, ip, ib] :
            gm.view<PowerUp, Position2D, Hitbox>().getVector()) {
            for (auto [player, state, health, pp, pb] :
                gm.view<Player, Health, Position2D, Hitbox>().getVector()) {
                if (state.dying || health.hp <= 0 ||
                    std::ranges::find(served, player) != served.end() ||
                    !touches(ip, ib, pp, pb))
                    continue;
                giveForce(gm, player, powerUp.laser);
                served.push_back(player);
                destroyNetworked(gm, item);
                break;
            }
        }
    }

} // namespace RType::Server
