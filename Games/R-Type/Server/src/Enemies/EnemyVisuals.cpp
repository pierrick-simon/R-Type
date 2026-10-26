/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.cpp
*/

#include "Enemies/EnemyVisuals.hpp"

#include "Clock.hpp"
#include "Position2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture.hpp"
#include "UpdateClientEntity.hpp"

namespace RType {

    SpriteSheet2D makeSheet(const SheetInfo &info, float scale = SPRITE_SCALE)
    {
        return SpriteSheet2D({1, info.frames}, info.frameMs, scale);
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
        gm.addComponent<Clock>(e);
        gm.addComponent<Lifetime>(e, Lifetime {duration});
        gm.addComponent<UpdateClientEntity>(
            e, EntityType::SPRITESHEET, info.asset, gm, e);
    }

    EnemyHooks makeEnemyVisualHooks()
    {
        EnemyHooks hooks;

        hooks.onEnemySpawn =
            [](GameManager &gm, Entity e, const EnemyDef &def) {
                gm.addComponent<SpriteSheet2D>(e, makeSheet(*def.sheet));
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

} // namespace RType
