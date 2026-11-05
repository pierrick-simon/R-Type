/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Force
*/

#include "Weapons/Force.hpp"

#include <algorithm>

#include "Assets.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Player/Destroy.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    Position2D forcePosition(const Position2D &player)
    {
        return {player.x + PLAYER_WIDTH - FORCE_OVERLAP,
            player.y + PLAYER_NOSE_Y - FORCE_HEIGHT / 2.f};
    }

    Hitbox forceHitbox(int level)
    {
        const auto &box = FORCE_HITBOXES[std::size_t(level - 1)];

        return {.width = box[2] * SCALE,
            .height = box[3] * SCALE,
            .offsetX = box[0] * SCALE,
            .offsetY = box[1] * SCALE};
    }

    std::optional<Entity> findForce(GameManager &gm, Entity player)
    {
        for (auto [entity, force] : gm.view<Force>().getVector())
            if (force.owner == player)
                return entity;
        return std::nullopt;
    }

    void spawnForce(GameManager &gm, Entity player, LaserType laser)
    {
        const Entity force = gm.createEntity();

        gm.addComponent<Force>(force, Force {player, 1, laser});
        gm.addComponent<ShotBlocker>(force);
        gm.addComponent<ContactDamage>(force,
            ContactDamage {FORCE_CONTACT_DAMAGE, FORCE_CONTACT_COOLDOWN});
        gm.addComponent<Hitbox>(force, forceHitbox(1));
        gm.addComponent<Position2D>(
            force, forcePosition(gm.getComponent<Position2D>(player)));
        gm.addComponent<Velocity2D>(force);
        gm.addComponent<SpriteSheet2D>(force,
            Size2D(FORCE_MAX_LEVEL, FORCE_FRAMES),
            FORCE_FRAME_MS,
            SCALE);
        gm.addComponent<UpdateClientEntity>(
            force, EntityType::SPRITESHEET, Asset::FORCE, gm, force);
    }

    void giveForce(GameManager &gm, Entity player, LaserType laser)
    {
        const std::optional<Entity> found = findForce(gm, player);

        if (!found) {
            spawnForce(gm, player, laser);
            return;
        }
        auto &force = gm.getComponent<Force>(*found);

        force.level = std::min(force.level + 1, FORCE_MAX_LEVEL);
        force.laser = laser;
        gm.getComponent<Hitbox>(*found) = forceHitbox(force.level);
    }

    void forceSystem(GameManager &gm)
    {
        for (auto [entity, force, pos, velocity] :
            gm.view<Force, Position2D, Velocity2D>().getVector()) {
            if (!gm.hasComponent<Player>(force.owner) ||
                gm.getComponent<Player>(force.owner).dying) {
                destroyNetworked(gm, entity);
                continue;
            }
            pos = forcePosition(gm.getComponent<Position2D>(force.owner));
            velocity = gm.getComponent<Velocity2D>(force.owner);
        }
    }

    void forceAnimation(const Force &force, SpriteSheet2D &sheet)
    {
        sheet.currentTile = std::size_t(force.level - 1) * FORCE_FRAMES +
            sheet.currentTile % FORCE_FRAMES;
    }

} // namespace RType::Server
