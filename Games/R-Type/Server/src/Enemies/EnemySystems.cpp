/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy movement systems
*/

#include "Enemies/EnemySystems.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

namespace RType {

    namespace {

        constexpr float PI = 3.14159265358979323846f;

        float elapsedSeconds(const Clock &clock)
        {
            return static_cast<float>(clock.getElapsed().count()) /
                static_cast<float>(MILLI_IN_SEC);
        }

        void applyStraightPattern(
            const MovementPattern &pattern, Velocity2D &velocity)
        {
            velocity.x = pattern.horizontalVelocity;
            velocity.y = 0.f;
        }

        void applySineWavePattern(
            MovementPattern &pattern, Velocity2D &velocity)
        {
            velocity.x = pattern.horizontalVelocity;
            if (pattern.period <= 0.f) {
                velocity.y = 0.f;
                return;
            }
            const float angularFrequency = 2.f * PI / pattern.period;

            velocity.y = pattern.amplitude * angularFrequency *
                std::cos(angularFrequency * pattern.elapsed + pattern.phase);
        }

        void applyZigZagPattern(
            MovementPattern &pattern, Velocity2D &velocity, float delta)
        {
            velocity.x = pattern.horizontalVelocity;
            if (pattern.period <= 0.f) {
                velocity.y = 0.f;
                return;
            }
            const float previousElapsed = pattern.elapsed - delta;
            const auto previousStep = static_cast<unsigned long long>(
                previousElapsed / pattern.period);
            const auto currentStep = static_cast<unsigned long long>(
                pattern.elapsed / pattern.period);

            if ((currentStep - previousStep) % 2 != 0)
                pattern.zigZagDirection *= -1.f;
            velocity.y = pattern.amplitude * pattern.zigZagDirection;
        }

    } // namespace

    void systemEnemyPattern(
        MovementPattern &pattern, Velocity2D &velocity, const Clock &clock)
    {
        const float delta = elapsedSeconds(clock);

        pattern.elapsed += delta;
        switch (pattern.type) {
            case PatternType::Straight:
                applyStraightPattern(pattern, velocity);
                break;
            case PatternType::SineWave:
                applySineWavePattern(pattern, velocity);
                break;
            case PatternType::ZigZag:
                applyZigZagPattern(pattern, velocity, delta);
                break;
        }
    }

    namespace {

        constexpr float OFFSCREEN_MARGIN = 150.f;

        struct Rect {
            float x;
            float y;
            float w;
            float h;
        };

        Rect rectOf(const Position2D &pos, const Hitbox &box)
        {
            return {pos.x + box.offsetX, pos.y + box.offsetY, box.width,
                box.height};
        }

        bool overlap(const Rect &a, const Rect &b)
        {
            return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h &&
                b.y < a.y + a.h;
        }

        /** Bullets disappear on every side, enemies only once they are gone. */
        bool leftScreen(const Rect &r, bool everySide)
        {
            if (r.x + r.w < -OFFSCREEN_MARGIN)
                return true;
            if (!everySide)
                return false;
            return r.x > SCREEN_WIDTH + OFFSCREEN_MARGIN ||
                r.y + r.h < -OFFSCREEN_MARGIN ||
                r.y > SCREEN_HEIGHT + OFFSCREEN_MARGIN;
        }

        /** Centre of the first EnemyTarget. False when there is none. */
        bool findTarget(GameManager &gm, float &x, float &y)
        {
            bool found = false;

            gm.view<EnemyTarget, Position2D, Hitbox>().each(
                [&](Entity, EnemyTarget &, Position2D &pos, Hitbox &box) {
                    if (found)
                        return;
                    x = pos.x + box.offsetX + box.width / 2.f;
                    y = pos.y + box.offsetY + box.height / 2.f;
                    found = true;
                });
            return found;
        }

        void killEnemy(GameManager &gm, const EnemyHooks &hooks, Entity enemy,
            const Position2D &pos, const Hitbox &box)
        {
            if (hooks.onEnemyDeath)
                hooks.onEnemyDeath(gm,
                    pos.x + box.offsetX + box.width / 2.f,
                    pos.y + box.offsetY + box.height / 2.f);
            gm.destroyEntity(enemy); // idempotent, applied at end of frame
        }

    } // namespace

    void systemEnemyWeapon(GameManager &gm, Weapon &weapon,
        const Position2D &pos, const Hitbox &hitbox, const Clock &clock,
        const EnemyHooks &hooks)
    {
        weapon.timer += elapsedSeconds(clock);
        if (weapon.timer < weapon.cooldown)
            return;
        // The shot leaves from the middle of the left edge of the hitbox.
        const float muzzleX = pos.x + hitbox.offsetX;
        const float muzzleY = pos.y + hitbox.offsetY + hitbox.height / 2.f;
        if (muzzleX > SCREEN_WIDTH)
            return;

        float dirX = -1.f;
        float dirY = 0.f;
        float targetX = 0.f;
        float targetY = 0.f;
        if (findTarget(gm, targetX, targetY)) {
            if (targetX >= muzzleX)
                return; // the player is behind us: keep the shot for later
            const float dx = targetX - muzzleX;
            const float dy = targetY - muzzleY;
            const float length = std::sqrt(dx * dx + dy * dy);
            dirX = dx / length;
            dirY = dy / length;
        }
        weapon.timer = 0.f;
        fireEnemyBullet(gm,
            muzzleX,
            muzzleY,
            dirX * weapon.bulletSpeed,
            dirY * weapon.bulletSpeed,
            weapon.damage,
            hooks);
    }

    void systemEnemyDirector(GameManager &gm, EnemyDirector &director,
        const Clock &clock, const EnemyHooks &hooks)
    {
        if (director.timeline.empty())
            return;
        director.elapsed += elapsedSeconds(clock);
        while (director.next < director.timeline.size() &&
            director.timeline[director.next].time <= director.elapsed) {
            const SpawnEvent &event = director.timeline[director.next++];

            spawnEnemy(gm,
                event.type,
                SCREEN_WIDTH + SPAWN_MARGIN,
                event.y,
                hooks);
        }
        if (director.next >= director.timeline.size() &&
            director.restartDelay >= 0.f &&
            director.elapsed >=
                director.timeline.back().time + director.restartDelay) {
            director.elapsed = 0.f;
            director.next = 0;
        }
    }

    void systemEnemyCleanup(
        GameManager &gm, EnemyDirector &, const Clock &clock)
    {
        const float delta = elapsedSeconds(clock);

        gm.view<Enemy, Position2D, Hitbox>().each(
            [&](Entity e, Enemy &, Position2D &pos, Hitbox &box) {
                if (leftScreen(rectOf(pos, box), false))
                    gm.destroyEntity(e);
            });
        gm.view<Projectile, Position2D, Hitbox>().each(
            [&](Entity e, Projectile &, Position2D &pos, Hitbox &box) {
                if (leftScreen(rectOf(pos, box), true))
                    gm.destroyEntity(e);
            });
        gm.view<Lifetime>().each([&](Entity e, Lifetime &life) {
            life.remaining -= delta;
            if (life.remaining <= 0.f)
                gm.destroyEntity(e);
        });
    }

    void systemEnemyCombat(
        GameManager &gm, EnemyDirector &director, const EnemyHooks &hooks)
    {
        // 1. Bullets.
        gm.view<Projectile, Position2D, Hitbox>().each(
            [&](Entity bullet, Projectile &shot, Position2D &bp, Hitbox &bb) {
                const Rect bulletRect = rectOf(bp, bb);
                bool consumed = false;

                if (shot.fromPlayer) {
                    gm.view<Enemy, Health, Position2D, Hitbox>().each(
                        [&](Entity enemy,
                            Enemy &info,
                            Health &health,
                            Position2D &ep,
                            Hitbox &eb) {
                            if (consumed || health.hp <= 0 ||
                                !overlap(bulletRect, rectOf(ep, eb)))
                                return;
                            consumed = true;
                            health.hp -= shot.damage;
                            if (health.hp > 0)
                                return;
                            director.score += info.scoreValue;
                            killEnemy(gm, hooks, enemy, ep, eb);
                        });
                } else {
                    gm.view<EnemyTarget, Health, Position2D, Hitbox>().each(
                        [&](Entity,
                            EnemyTarget &,
                            Health &health,
                            Position2D &tp,
                            Hitbox &tb) {
                            if (consumed || health.hp <= 0 ||
                                !overlap(bulletRect, rectOf(tp, tb)))
                                return;
                            consumed = true;
                            health.hp = std::max(0, health.hp - shot.damage);
                        });
                }
                if (consumed)
                    gm.destroyEntity(bullet);
            });

        // 2. Ramming: the enemy explodes (no score) and hurts the target once.
        gm.view<Enemy, Health, Position2D, Hitbox>().each(
            [&](Entity enemy,
                Enemy &info,
                Health &health,
                Position2D &ep,
                Hitbox &eb) {
                if (health.hp <= 0)
                    return;
                const Rect enemyRect = rectOf(ep, eb);

                gm.view<EnemyTarget, Health, Position2D, Hitbox>().each(
                    [&](Entity,
                        EnemyTarget &,
                        Health &targetHealth,
                        Position2D &tp,
                        Hitbox &tb) {
                        if (health.hp <= 0 || targetHealth.hp <= 0 ||
                            !overlap(enemyRect, rectOf(tp, tb)))
                            return;
                        targetHealth.hp = std::max(
                            0, targetHealth.hp - info.contactDamage);
                        health.hp = 0;
                        killEnemy(gm, hooks, enemy, ep, eb);
                    });
            });
    }

    void registerEnemySystems(GameManager &gm, EnemyHooks hooks)
    {
        const auto h = std::make_shared<const EnemyHooks>(std::move(hooks));

        gm.createSystem<MovementPattern, Velocity2D, Clock>(systemEnemyPattern);
        gm.createSystem<Weapon, Position2D, Hitbox, Clock>(
            [h](GameManager &g,
                Weapon &w,
                Position2D &p,
                Hitbox &b,
                Clock &c) { systemEnemyWeapon(g, w, p, b, c, *h); });
        gm.createSystem<EnemyDirector, Clock>(
            [h](GameManager &g, EnemyDirector &d, Clock &c) {
                systemEnemyDirector(g, d, c, *h);
            });
        gm.createSystem<EnemyDirector, Clock>(
            [](GameManager &g, EnemyDirector &d, Clock &c) {
                systemEnemyCleanup(g, d, c);
            });
        gm.createSystem<EnemyDirector>([h](GameManager &g, EnemyDirector &d) {
            systemEnemyCombat(g, d, *h);
        });
    }

} // namespace RType
