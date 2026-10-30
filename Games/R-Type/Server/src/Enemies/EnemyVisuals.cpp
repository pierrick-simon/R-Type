/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.cpp
*/

#include "Enemies/EnemyVisuals.hpp"

#include "Components.hpp"
#include "Position2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture.hpp"
#include "UpdateClientEntity.hpp"

namespace RType {

    SpriteSheet2D makeSheet(const SheetInfo &info, float scale = SPRITE_SCALE)
    {
        return SpriteSheet2D(
            {info.rows, info.frames / info.rows}, info.frameMs, scale);
    }

    void systemWalkerAnimation(const Walker &walker, SpriteSheet2D &sheet)
    {
        const WalkerFrames &frames = WALKER_FRAMES[walker.facingLeft ? 0 : 1];

        switch (walker.state) {
            case WalkerState::Crouch:
            case WalkerState::Land:
                sheet.currentTile = frames.crouch;
                break;
            case WalkerState::Jump: {
                const auto step = std::size_t(
                    walker.timer * 2000.f / float(WALKER_SHEET.frameMs));
                sheet.currentTile = frames.jump[step % frames.jump.size()];
                break;
            }
            case WalkerState::Walk: {
                const auto step = std::size_t(
                    walker.stride * 1000.f / float(WALKER_SHEET.frameMs));
                sheet.currentTile = frames.walk[step % frames.walk.size()];
                break;
            }
        }
    }

    void spawnExplosion(GameManager &gm, float cx, float cy)
    {
        const SheetInfo &info = EXPLOSION_SHEET;
        const Entity e = gm.createEntity();
        const float duration =
            float(info.frames * (info.frameMs + 17)) / 1000.f;

        gm.addComponent<Position2D>(e,
            Position2D {cx - float(info.cellW) * SPRITE_SCALE / 2.f,
                cy - float(info.cellH) * SPRITE_SCALE / 2.f});
        gm.addComponent<SpriteSheet2D>(e, makeSheet(info));
        gm.addComponent<Server::PlayOnce>(e);
        gm.addComponent<Lifetime>(e, Lifetime {duration});
        gm.addComponent<UpdateClientEntity>(
            e, EntityType::SPRITESHEET, info.asset, gm, e);
    }

    EnemyHooks makeEnemyVisualHooks()
    {
        EnemyHooks hooks;

        hooks.onEnemySpawn =
            [](GameManager &gm, Entity e, const EnemyDef &def) {
                SpriteSheet2D sheet = makeSheet(*def.sheet);

                sheet.animation = def.pattern != PatternType::WalkJump;
                gm.addComponent<SpriteSheet2D>(e, sheet);
                gm.addComponent<UpdateClientEntity>(
                    e, EntityType::SPRITESHEET, def.sheet->asset, gm, e);
            };
        hooks.onBulletSpawn = [](GameManager &gm, Entity e) {
            gm.addComponent<SpriteSheet2D>(e, makeSheet(ENEMY_BULLET_SHEET));
            gm.addComponent<UpdateClientEntity>(
                e, EntityType::SPRITESHEET, ENEMY_BULLET_SHEET.asset, gm, e);
        };
        hooks.onEnemyDeath = spawnExplosion;
        return hooks;
    }

    void registerEnemyVisualSystems(GameManager &gm)
    {
        gm.createSystem<Walker, SpriteSheet2D>(systemWalkerAnimation);
    }

} // namespace RType
