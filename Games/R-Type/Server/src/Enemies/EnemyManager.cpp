/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyManager.cpp
*/

#include "Enemies/EnemyManager.hpp"
#include "Player/Destroy.hpp"

#include <algorithm>

#include "Common/Terrain.hpp"
#include "Position2D.hpp"
#include "Velocity2D.hpp"

namespace RType {

    namespace {
        Hitbox centeredHitbox(const SheetInfo &sheet)
        {
            Hitbox hitbox = {0};

            hitbox.width = float(sheet.hitboxW) * sheet.scale;
            hitbox.height = float(sheet.hitboxH) * sheet.scale;
            hitbox.offsetX =
                float(sheet.cellW - sheet.hitboxW) * sheet.scale / 2.f;
            hitbox.offsetY =
                float(sheet.cellH - sheet.hitboxH) * sheet.scale / 2.f;
            return hitbox;
        }

        Hitbox bossHitbox()
        {
            return {.width = BOSS_DEF.hitboxWidth * LEVEL_SCALE,
                .height = BOSS_DEF.hitboxHeight * LEVEL_SCALE,
                .offsetX = BOSS_DEF.hitboxLeft * LEVEL_SCALE,
                .offsetY = BOSS_DEF.hitboxTop * LEVEL_SCALE};
        }

        template<typename Component>
        void destroyAll(GameManager &gm)
        {
            std::vector<Entity> entities;

            gm.view<Component>().each(
                [&entities](Entity e, Component &) { entities.push_back(e); });
            for (const Entity e : entities)
                destroyNetworked(gm, e);
        }
    } // namespace

    void EnemyDirector::addWave(
        EnemyType type, float y, int count, float interval, float start)
    {
        for (int i = 0; i < count; ++i)
            timeline.push_back({start + float(i) * interval, type, y});
        std::stable_sort(timeline.begin(),
            timeline.end(),
            [](const SpawnEvent &a, const SpawnEvent &b) {
                return a.time < b.time;
            });
    }

    Entity spawnEnemy(GameManager &gm, EnemyType type, float x, float y,
        const EnemyHooks &hooks)
    {
        const EnemyDef &def = enemyDef(type);
        const SheetInfo &sheet = *def.sheet;
        const Entity entity = gm.createEntity();

        MovementPattern pattern;
        pattern.type = def.pattern;
        pattern.horizontalVelocity = -def.speed;
        pattern.amplitude = def.amplitude;
        pattern.period = def.period;

        const Hitbox hitbox = def.pattern == PatternType::Boss
            ? bossHitbox()
            : centeredHitbox(sheet);

        if (def.pattern == PatternType::WalkJump) {
            const auto platform =
                Server::platformAt(gm, x + hitbox.offsetX + hitbox.width / 2.f);

            if (platform)
                y = platform->top - float(sheet.cellH) * SPRITE_SCALE;
        }
        gm.addComponent<Enemy>(entity, Enemy {type, def.scoreValue});
        gm.addComponent<Health>(entity, Health {def.hp, def.hp});
        gm.addComponent<Position2D>(entity, Position2D {x, y});
        gm.addComponent<Velocity2D>(entity);
        if (def.pattern == PatternType::WalkJump)
            gm.addComponent<Walker>(entity);
        else if (def.pattern == PatternType::AlienBeam)
            gm.addComponent<Alien>(entity);
        else if (def.pattern == PatternType::Boss)
            gm.addComponent<Boss>(entity);
        else
            gm.addComponent<MovementPattern>(entity, pattern);
        gm.addComponent<Hitbox>(entity, hitbox);
        if (def.fireCooldown > 0.f) {
            Weapon weapon;
            weapon.cooldown = def.fireCooldown;
            weapon.bulletSpeed = def.bulletSpeed;
            weapon.damage = def.bulletDamage;
            gm.addComponent<Weapon>(entity, weapon);
        }
        if (hooks.onEnemySpawn)
            hooks.onEnemySpawn(gm, entity, def);
        return entity;
    }

    Entity fireEnemyBullet(GameManager &gm, float x, float y, float vx,
        float vy, int damage, const EnemyHooks &hooks, const SheetInfo &sheet)
    {
        const Entity bullet = gm.createEntity();

        gm.addComponent<Position2D>(bullet,
            Position2D {x - float(sheet.cellW) * sheet.scale / 2.f,
                y - float(sheet.cellH) * sheet.scale / 2.f});
        gm.addComponent<Velocity2D>(bullet, Velocity2D {vx, vy});
        gm.addComponent<Projectile>(bullet, Projectile {damage, false});
        gm.addComponent<Hitbox>(bullet, centeredHitbox(sheet));
        if (hooks.onBulletSpawn)
            hooks.onBulletSpawn(gm, bullet, sheet);
        return bullet;
    }

    Entity fireAlienBeam(GameManager &gm, float x, float y, bool facingLeft,
        const EnemyHooks &hooks)
    {
        const Entity beam = gm.createEntity();
        const SheetInfo &sheet = ALIEN_BEAM_SHEET;
        const float width = float(sheet.cellW) * SPRITE_SCALE;
        const float head = float(sheet.cellH) * SPRITE_SCALE / 2.f;
        Hitbox hitbox = centeredHitbox(sheet);

        hitbox.offsetX = facingLeft ? 0.f : width - hitbox.width;
        gm.addComponent<Position2D>(beam,
            Position2D {facingLeft ? x - head : x - width + head, y - head});
        gm.addComponent<Velocity2D>(beam,
            Velocity2D {
                facingLeft ? -ALIEN_DEF.beamSpeed : ALIEN_DEF.beamSpeed, 0.f});
        gm.addComponent<Projectile>(
            beam, Projectile {ALIEN_DEF.beamDamage, false});
        gm.addComponent<Hitbox>(beam, hitbox);
        if (hooks.onBeamSpawn)
            hooks.onBeamSpawn(gm, beam, facingLeft);
        return beam;
    }

    void clearEnemies(GameManager &gm)
    {
        destroyAll<Enemy>(gm);
        destroyAll<Projectile>(gm);
        destroyAll<Lifetime>(gm);
    }

    void resetEnemies(GameManager &gm)
    {
        clearEnemies(gm);
        gm.view<EnemyDirector>().each([](Entity, EnemyDirector &director) {
            director.elapsed = 0.f;
            director.next = 0;
            director.score = 0;
            director.bossSpawned = false;
            director.victoryTimer = 0.f;
            director.levelCleared = false;
            director.paused = false;
        });
    }

    bool levelCleared(GameManager &gm)
    {
        return std::ranges::any_of(gm.view<EnemyDirector>().getVector(),
            [](const auto &entry) { return std::get<1>(entry).levelCleared; });
    }

    Entity createEnemyDirector(GameManager &gm)
    {
        const Entity entity = gm.createEntity();
        EnemyDirector director;

        director.addWave(EnemyType::Scout, 300.f, 5, 0.5f, 2.f);
        director.addWave(EnemyType::Walker, 0.f, 1, 0.f, 4.f);
        director.addWave(EnemyType::Scout, 800.f, 5, 0.5f, 6.f);
        director.addWave(EnemyType::Alien, 350.f, 1, 0.f, 8.f);
        director.addWave(EnemyType::Gunner, 540.f, 2, 1.5f, 10.f);
        director.addWave(EnemyType::Scout, 540.f, 8, 0.4f, 15.f);
        gm.addComponent<EnemyDirector>(entity, std::move(director));
        return entity;
    }

} // namespace RType
