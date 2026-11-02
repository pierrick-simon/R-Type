/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.cpp
*/

#include "Enemies/EnemyVisuals.hpp"

#include <algorithm>

#include "Components.hpp"
#include "Destroy.hpp"
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

    void systemAlienAnimation(const Alien &alien, SpriteSheet2D &sheet)
    {
        sheet.currentTile = alienTile(alien);
        sheet.flip = !alien.facingLeft;
    }

    /** @brief Glow growing at the gun tip of an alien charging its beam. */
    struct AlienCharge {
        Entity alien;
        float duration = 0.f;
    };

    void placeAlienCharge(GameManager &gm, const AlienCharge &charge,
        const Lifetime &life, SpriteSheet2D &sheet, Position2D &pos)
    {
        const float progress =
            std::clamp(1.f - life.remaining / charge.duration, 0.f, 1.f);
        const auto [x, y] = alienMuzzle(gm.getComponent<Alien>(charge.alien),
            gm.getComponent<Position2D>(charge.alien));
        sheet.scale = SPRITE_SCALE * (0.3f + 0.9f * progress);
        const float half = float(ALIEN_CHARGE_SHEET.cellW) * sheet.scale / 2.f;

        pos = Position2D {x - half, y - half};
    }

    void systemAlienCharge(GameManager &gm)
    {
        gm.view<AlienCharge, Lifetime, SpriteSheet2D, Position2D>().each(
            [&gm](Entity e,
                AlienCharge &charge,
                Lifetime &life,
                SpriteSheet2D &sheet,
                Position2D &pos) {
                if (!gm.hasComponent<Alien>(charge.alien)) {
                    destroyNetworked(gm, e);
                    return;
                }
                placeAlienCharge(gm, charge, life, sheet, pos);
            });
    }

    void spawnAlienCharge(GameManager &gm, Entity alien, float duration)
    {
        const Entity e = gm.createEntity();
        const AlienCharge charge {alien, duration};
        const Lifetime life {duration};
        SpriteSheet2D sheet = makeSheet(ALIEN_CHARGE_SHEET);
        Position2D pos;

        sheet.animation = false;
        placeAlienCharge(gm, charge, life, sheet, pos);
        gm.addComponent<Position2D>(e, pos);
        gm.addComponent<SpriteSheet2D>(e, sheet);
        gm.addComponent<AlienCharge>(e, charge);
        gm.addComponent<Lifetime>(e, life);
        gm.addComponent<UpdateClientEntity>(
            e, EntityType::SPRITESHEET, ALIEN_CHARGE_SHEET.asset, gm, e);
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
        gm.addComponent<SoundInfo>(e, SoundInfo::PLAY);
        gm.addComponent<UpdateClientEntity>(e,
            EntityType::SPRITESHEET,
            info.asset,
            gm,
            e,
            Asset::EXPLOSION_SOUND);
    }

    EnemyHooks makeEnemyVisualHooks()
    {
        EnemyHooks hooks;

        hooks.onEnemySpawn =
            [](GameManager &gm, Entity e, const EnemyDef &def) {
                SpriteSheet2D sheet = makeSheet(*def.sheet);

                sheet.animation = def.pattern != PatternType::WalkJump &&
                    def.pattern != PatternType::AlienBeam;
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
        hooks.onAlienCharge = spawnAlienCharge;
        hooks.onBeamSpawn = [](GameManager &gm, Entity e, bool facingLeft) {
            SpriteSheet2D sheet = makeSheet(ALIEN_BEAM_SHEET);

            sheet.flip = !facingLeft;
            gm.addComponent<SpriteSheet2D>(e, sheet);
            gm.addComponent<UpdateClientEntity>(
                e, EntityType::SPRITESHEET, ALIEN_BEAM_SHEET.asset, gm, e);
        };
        return hooks;
    }

    void registerEnemyVisualSystems(GameManager &gm)
    {
        gm.createSystem<Walker, SpriteSheet2D>(systemWalkerAnimation);
        gm.createSystem<Alien, SpriteSheet2D>(systemAlienAnimation);
        gm.createSystem<>(systemAlienCharge);
    }

} // namespace RType
