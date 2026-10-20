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
#include "Texture2D.hpp"

namespace RType {

    SpriteSheet2D makeSheet(const SheetInfo &info, float scale = SPRITE_SCALE)
    {
        SpriteSheet2D ss = {};
        ss.recNb() = {1, info.frames};
        ss.frameDuration() = std::chrono::milliseconds(info.frameMs);
        ss.scale() = scale;
        return ss;
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
        gm.addComponent<Texture2D>(e, Server::loadPixelTexture(gm, info.asset));
        gm.addComponent<SpriteSheet2D>(e, makeSheet(info));
        gm.addComponent<Clock>(e);
        gm.addComponent<Lifetime>(e, Lifetime {duration});
    }

    EnemyHooks makeEnemyVisualHooks()
    {
        EnemyHooks hooks;

        hooks.onEnemySpawn =
            [](GameManager &gm, Entity e, const EnemyDef &def) {
                gm.addComponent<Texture2D>(
                    e, Server::loadPixelTexture(gm, def.sheet->asset));
                gm.addComponent<SpriteSheet2D>(e, makeSheet(*def.sheet));
            };
        hooks.onBulletSpawn = [](GameManager &gm, Entity e) {
            gm.addComponent<Texture2D>(
                e, Server::loadPixelTexture(gm, ENEMY_BULLET_SHEET.asset));
            gm.addComponent<SpriteSheet2D>(e, makeSheet(ENEMY_BULLET_SHEET));
        };
        hooks.onEnemyDeath = spawnExplosion;
        return hooks;
    }

} // namespace RType
