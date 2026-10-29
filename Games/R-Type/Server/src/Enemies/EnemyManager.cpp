/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyManager.cpp
*/

#include "Enemies/EnemyManager.hpp"
#include "Destroy.hpp"

#include <algorithm>

#include "Position2D.hpp"
#include "Velocity2D.hpp"

namespace RType {

    namespace {
        Hitbox centeredHitbox(const SheetInfo &sheet)
        {
            Hitbox hitbox = {0};

            hitbox.width = float(sheet.hitboxW) * SPRITE_SCALE;
            hitbox.height = float(sheet.hitboxH) * SPRITE_SCALE;
            hitbox.offsetX =
                float(sheet.cellW - sheet.hitboxW) * SPRITE_SCALE / 2.f;
            hitbox.offsetY =
                float(sheet.cellH - sheet.hitboxH) * SPRITE_SCALE / 2.f;
            return hitbox;
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

        const Hitbox hitbox = centeredHitbox(sheet);

        gm.addComponent<Enemy>(entity, Enemy {type, def.scoreValue});
        gm.addComponent<Health>(entity, Health {def.hp, def.hp});
        gm.addComponent<Position2D>(entity, Position2D {x, y});
        gm.addComponent<Velocity2D>(entity);
        gm.addComponent<MovementPattern>(entity, pattern);
        gm.addComponent<Hitbox>(entity, hitbox);
        if (type == EnemyType::Gunner && def.fireCooldown > 0.f) {
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
        float vy, int damage, const EnemyHooks &hooks)
    {
        const Entity bullet = gm.createEntity();
        const SheetInfo &sheet = ENEMY_BULLET_SHEET;

        gm.addComponent<Position2D>(bullet,
            Position2D {x - float(sheet.cellW) * SPRITE_SCALE / 2.f,
                y - float(sheet.cellH) * SPRITE_SCALE / 2.f});
        gm.addComponent<Velocity2D>(bullet, Velocity2D {vx, vy});
        gm.addComponent<Projectile>(bullet, Projectile {damage, false});
        gm.addComponent<Hitbox>(bullet, centeredHitbox(sheet));
        if (hooks.onBulletSpawn)
            hooks.onBulletSpawn(gm, bullet);
        return bullet;
    }

    void resetEnemies(GameManager &gm)
    {
        destroyAll<Enemy>(gm);
        destroyAll<Projectile>(gm);
        destroyAll<Lifetime>(gm);
        gm.view<EnemyDirector>().each([](Entity, EnemyDirector &director) {
            director.elapsed = 0.f;
            director.next = 0;
            director.score = 0;
        });
    }

    Entity createEnemyDirector(GameManager &gm)
    {
        const Entity entity = gm.createEntity();
        EnemyDirector director;

        director.addWave(EnemyType::Scout, 300.f, 5, 0.5f, 2.f);
        director.addWave(EnemyType::Scout, 800.f, 5, 0.5f, 6.f);
        director.addWave(EnemyType::Gunner, 540.f, 2, 1.5f, 10.f);
        director.addWave(EnemyType::Scout, 540.f, 8, 0.4f, 15.f);
        gm.addComponent<EnemyDirector>(entity, std::move(director));
        return entity;
    }

} // namespace RType
