/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyVisuals.cpp
*/

#include "Enemies/EnemyVisuals.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

#include "Components.hpp"
#include "Player/Destroy.hpp"
#include "Position2D.hpp"
#include "SpriteSheet2D.hpp"
#include "UpdateClientEntity.hpp"

namespace RType {

    SpriteSheet2D makeSheet(const SheetInfo &info)
    {
        const int rows = info.flashFrames ? info.rows * 2 : info.rows;

        return SpriteSheet2D(
            {rows, info.frames / info.rows}, info.frameMs, info.scale);
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

    bool isHurtBlinking(const Health &health)
    {
        return health.hurtTimer > 0.f &&
            int(health.hurtTimer / ENEMY_HURT_BLINK) % 2 == 0;
    }

    void systemEnemyHurtBlink(GameManager &gm)
    {
        gm.view<Enemy, Health, SpriteSheet2D>().each(
            [](Entity, Enemy &enemy, Health &health, SpriteSheet2D &sheet) {
                const SheetInfo &info = *enemyDef(enemy.type).sheet;

                if (!info.flashFrames)
                    return;
                const auto frames = std::size_t(info.frames);

                sheet.currentTile = sheet.currentTile % frames +
                    (isHurtBlinking(health) ? frames : 0);
            });
    }

    void systemBossAnimation(
        const Boss &boss, const Health &health, SpriteSheet2D &sheet)
    {
        const bool flash =
            boss.state == BossState::Dying && int(boss.timer / 0.08f) % 2 != 0;

        const bool hurt = isHurtBlinking(health);

        // The row is how far the belly is open, the columns an idle loop.
        sheet.currentTile = std::size_t(boss.stage * BOSS_IDLE_FRAMES) +
            sheet.currentTile % BOSS_IDLE_FRAMES +
            (hurt ? BOSS_FLASH_OFFSET : 0);
        sheet.color =
            flash ? raylib::Color {255, 90, 90, 255} : raylib::Color::White();
    }

    void systemBossTailAnimation(const BossTail &tail, SpriteSheet2D &sheet)
    {
        constexpr float step = std::numbers::pi_v<float> / 8.f;

        if (tail.index != BOSS_TAIL_SEGMENTS - 1)
            return;
        const auto frame = int(
            std::lround((tail.angle - std::numbers::pi_v<float> / 2.f) / step));

        sheet.currentTile = std::size_t(
            std::clamp(frame, BOSS_TAIL_TIP_DOWN, BOSS_TAIL_TIP_UP));
    }

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

    void spawnExplosion(
        GameManager &gm, const SheetInfo &info, float cx, float cy)
    {
        const Entity e = gm.createEntity();
        const float duration =
            float(info.frames * (info.frameMs + 17)) / 1000.f;

        gm.addComponent<Position2D>(e,
            Position2D {cx - float(info.cellW) * info.scale / 2.f,
                cy - float(info.cellH) * info.scale / 2.f});
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
        hooks.onBulletSpawn =
            [](GameManager &gm, Entity e, const SheetInfo &info) {
                gm.addComponent<SpriteSheet2D>(e, makeSheet(info));
                gm.addComponent<UpdateClientEntity>(
                    e, EntityType::SPRITESHEET, info.asset, gm, e);
            };
        hooks.onBossPartSpawn = hooks.onBulletSpawn;
        hooks.onEnemyDeath = [](GameManager &gm, float x, float y) {
            spawnExplosion(gm, EXPLOSION_SHEET, x, y);
        };
        hooks.onBossExplosion = [](GameManager &gm, float x, float y) {
            spawnExplosion(gm, BOSS_EXPLOSION_SHEET, x, y);
        };
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
        gm.createSystem<BossTail, SpriteSheet2D>(systemBossTailAnimation);
        gm.createSystem<Boss, Health, SpriteSheet2D>(systemBossAnimation);
        gm.createSystem<>(systemEnemyHurtBlink);
        gm.createSystem<>(systemAlienCharge);
    }

} // namespace RType
