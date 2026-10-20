/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.cpp
*/

#include "Enemies/EnemyVisuals.hpp"

#include "Clock.hpp"
#include "Position2D.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    SpriteSheet2D makeSheet(const SheetInfo &info, float scale = SPRITE_SCALE)
    {
        // Size2D is {height, width}: one row, `frames` columns.
        // frameDurationTime must be set at construction: frameDuration is
        // computed from it by its default member initializer.
        return SpriteSheet2D {
            .recNb = {1, info.frames},
            .currentTile = 0,
            .frameDurationTime = static_cast<unsigned int>(info.frameMs),
            .origin = {},
            .scale = scale,
        };
    }

    void spawnExplosion(GameManager &gm, float cx, float cy)
    {
        const SheetInfo &info = EXPLOSION_SHEET;
        const Entity e = gm.createEntity();
        // One pass of the animation, plus a margin for the frame timing.
        const float duration =
            float(info.frames * (info.frameMs + 17)) / 1000.f;

        gm.addComponent<Position2D>(e,
            Position2D {cx - float(info.cellW) * SPRITE_SCALE / 2.f,
                cy - float(info.cellH) * SPRITE_SCALE / 2.f});
        gm.addComponent<Texture2D>(e, info.file);
        gm.addComponent<SpriteSheet2D>(e, makeSheet(info));
        gm.addComponent<Clock>(e);
        gm.addComponent<Lifetime>(e, Lifetime {duration});
    }

    EnemyHooks makeEnemyVisualHooks()
    {
        EnemyHooks hooks;

        hooks.onEnemySpawn =
            [](GameManager &gm, Entity e, const EnemyDef &def) {
                gm.addComponent<Texture2D>(e, def.sheet->file);
                gm.addComponent<SpriteSheet2D>(e, makeSheet(*def.sheet));
            };
        hooks.onBulletSpawn = [](GameManager &gm, Entity e) {
            gm.addComponent<Texture2D>(e, ENEMY_BULLET_SHEET.file);
            gm.addComponent<SpriteSheet2D>(e, makeSheet(ENEMY_BULLET_SHEET));
        };
        hooks.onEnemyDeath = spawnExplosion;
        return hooks;
    }

} // namespace RType
