/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy movement systems
*/

#include "Enemies/EnemySystems.hpp"
#include "Config.hpp"
#include "Destroy.hpp"
#include "Terrain.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

namespace RType {

    namespace {

        float elapsedSeconds(const MasterClock &clock)
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

    void systemEnemyPattern(MovementPattern &pattern, Velocity2D &velocity,
        const MasterClock &clock)
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
            case PatternType::WalkJump:
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
            return {pos.x + box.offsetX,
                pos.y + box.offsetY,
                box.width,
                box.height};
        }

        bool overlap(const Rect &a, const Rect &b)
        {
            return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h &&
                b.y < a.y + a.h;
        }

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

        bool findTarget(GameManager &gm, float &x, float &y)
        {
            bool found = false;

            gm.view<EnemyTarget, Health, Position2D, Hitbox>().each(
                [&](Entity,
                    EnemyTarget &,
                    Health &health,
                    Position2D &pos,
                    Hitbox &box) {
                    if (found || health.hp <= 0)
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
            destroyNetworked(gm, enemy);
        }

        void fireToward(GameManager &gm, Weapon &weapon, float x, float y,
            float targetX, float targetY, const EnemyHooks &hooks)
        {
            const float dx = targetX - x;
            const float dy = targetY - y;
            const float length = std::sqrt(dx * dx + dy * dy);

            if (length <= 0.f)
                return;
            weapon.timer = 0.f;
            fireEnemyBullet(gm,
                x,
                y,
                dx / length * weapon.bulletSpeed,
                dy / length * weapon.bulletSpeed,
                weapon.damage,
                hooks);
        }

    } // namespace

    void systemEnemyWeapon(GameManager &gm, const Enemy &enemy, Weapon &weapon,
        const Position2D &pos, const Hitbox &hitbox, const MasterClock &clock,
        const EnemyHooks &hooks)
    {
        if (enemy.type != EnemyType::Gunner)
            return;
        weapon.timer += elapsedSeconds(clock);
        if (weapon.timer < weapon.cooldown)
            return;
        const float muzzleX = pos.x + hitbox.offsetX;
        const float muzzleY = pos.y + hitbox.offsetY + hitbox.height / 2.f;
        if (muzzleX > SCREEN_WIDTH)
            return;

        float targetX = muzzleX - 1.f;
        float targetY = muzzleY;
        if (findTarget(gm, targetX, targetY) && targetX >= muzzleX)
            return;
        fireToward(gm, weapon, muzzleX, muzzleY, targetX, targetY, hooks);
    }

    namespace {

        // On the ground, the walker moves relative to the scrolling floor.
        constexpr float FLOOR_VELOCITY = -Server::BACKGROUND_SPEED;
        constexpr float WALKER_HEIGHT =
            float(WALKER_SHEET.cellH) * SPRITE_SCALE;

        float centerX(const Position2D &pos, const Hitbox &hitbox)
        {
            return pos.x + hitbox.offsetX + hitbox.width / 2.f;
        }

        void setWalkerState(Walker &walker, WalkerState state)
        {
            walker.state = state;
            walker.timer = 0.f;
        }

        struct Ledge {
            Server::Platform current;
            Server::Platform next;
            float edge;
        };

        /** @brief Platform under the walker and the one past its front. */
        std::optional<Ledge> findLedge(
            GameManager &gm, const Walker &walker, float x)
        {
            const float dir = walker.facingLeft ? -1.f : 1.f;
            const auto current = Server::platformAt(gm, x);

            if (!current)
                return std::nullopt;
            const float edge =
                walker.facingLeft ? current->left : current->right;
            const auto next = Server::platformAt(gm, edge + dir);

            if (!next)
                return std::nullopt;
            return Ledge {*current, *next, edge};
        }

        void walkerWalk(GameManager &gm, Walker &walker, Velocity2D &velocity,
            Position2D &pos, const Hitbox &hitbox)
        {
            const float dir = walker.facingLeft ? -1.f : 1.f;
            const float x = centerX(pos, hitbox);
            const auto ledge = findLedge(gm, walker, x);

            velocity.x += dir * enemyDef(EnemyType::Walker).speed;
            if (!ledge)
                return;
            pos.y = ledge->current.top - WALKER_HEIGHT;
            const float front = x + dir * hitbox.width / 2.f;
            if (walker.facingLeft ? front > ledge->edge : front < ledge->edge)
                return;
            const float drop = ledge->next.top - ledge->current.top;
            if (drop > WALKER_DEF.stepHeight) {
                velocity.x = FLOOR_VELOCITY;
                setWalkerState(walker, WalkerState::Crouch);
            } else if (drop < -WALKER_DEF.stepHeight &&
                walker.timer >= WALKER_DEF.turnDelay) {
                walker.facingLeft = !walker.facingLeft;
                setWalkerState(walker, WalkerState::Walk);
            }
        }

        /**
         * @brief Jumps down onto the lower platform in front of the walker,
         * along a ballistic arc landing a bit past the edge.
         */
        void startWalkerJump(GameManager &gm, Walker &walker,
            Velocity2D &velocity, const Position2D &pos, const Hitbox &hitbox)
        {
            const float dir = walker.facingLeft ? -1.f : 1.f;
            const float x = centerX(pos, hitbox);
            const auto ledge = findLedge(gm, walker, x);

            if (!ledge || ledge->next.top <= ledge->current.top) {
                setWalkerState(walker, WalkerState::Walk);
                return;
            }
            const float half = hitbox.width / 2.f;
            float landX =
                ledge->edge + dir * hitbox.width * WALKER_DEF.landingDistance;
            if (ledge->next.right - ledge->next.left < hitbox.width)
                landX = (ledge->next.left + ledge->next.right) / 2.f;
            else
                landX = std::clamp(
                    landX, ledge->next.left + half, ledge->next.right - half);
            const float g = WALKER_DEF.gravity;
            const float drop = ledge->next.top - ledge->current.top;
            const float vy = -std::sqrt(2.f * g * WALKER_DEF.jumpApex);
            const float duration =
                (-vy + std::sqrt(vy * vy + 2.f * g * drop)) / g;

            // The landing spot scrolls with the floor during the jump.
            walker.jumpVelocityX = (landX - x) / duration + FLOOR_VELOCITY;
            walker.landY = ledge->next.top;
            velocity = {walker.jumpVelocityX, vy};
            setWalkerState(walker, WalkerState::Jump);
        }

    } // namespace

    void systemWalkerMovement(GameManager &gm, Walker &walker,
        Velocity2D &velocity, Position2D &pos, const Hitbox &hitbox,
        const MasterClock &clock)
    {
        const float delta = elapsedSeconds(clock);

        walker.timer += delta;
        switch (walker.state) {
            case WalkerState::Walk:
                velocity = {FLOOR_VELOCITY, 0.f};
                walker.stride += delta;
                walkerWalk(gm, walker, velocity, pos, hitbox);
                break;
            case WalkerState::Crouch:
                velocity = {FLOOR_VELOCITY, 0.f};
                if (walker.timer >= WALKER_DEF.crouchTime)
                    startWalkerJump(gm, walker, velocity, pos, hitbox);
                break;
            case WalkerState::Jump:
                velocity.x = walker.jumpVelocityX;
                velocity.y += WALKER_DEF.gravity * delta;
                if (velocity.y > 0.f && pos.y + WALKER_HEIGHT >= walker.landY) {
                    pos.y = walker.landY - WALKER_HEIGHT;
                    velocity = {FLOOR_VELOCITY, 0.f};
                    setWalkerState(walker, WalkerState::Land);
                }
                break;
            case WalkerState::Land:
                velocity = {FLOOR_VELOCITY, 0.f};
                if (walker.timer >= WALKER_DEF.crouchTime)
                    setWalkerState(walker, WalkerState::Walk);
                break;
        }
    }

    void systemWalkerWeapon(GameManager &gm, Walker &walker, Weapon &weapon,
        const Position2D &pos, const Hitbox &hitbox, const MasterClock &clock,
        const EnemyHooks &hooks)
    {
        float targetX = 0.f;
        float targetY = 0.f;

        if (walker.state == WalkerState::Crouch ||
            walker.state == WalkerState::Land)
            return;
        weapon.timer += elapsedSeconds(clock);
        if (weapon.timer < weapon.cooldown || !findTarget(gm, targetX, targetY))
            return;
        const bool targetLeft = targetX < centerX(pos, hitbox);
        const float muzzleX =
            pos.x + hitbox.offsetX + (targetLeft ? 0.f : hitbox.width);
        const float muzzleY = pos.y + hitbox.offsetY + hitbox.height / 2.f;
        if (muzzleX < 0.f || muzzleX > SCREEN_WIDTH)
            return;
        fireToward(gm, weapon, muzzleX, muzzleY, targetX, targetY, hooks);
    }

    void systemEnemyDirector(GameManager &gm, EnemyDirector &director,
        const MasterClock &clock, const EnemyHooks &hooks)
    {
        if (director.timeline.empty())
            return;
        director.elapsed += elapsedSeconds(clock);
        while (director.next < director.timeline.size() &&
            director.timeline[director.next].time <= director.elapsed) {
            const SpawnEvent &event = director.timeline[director.next++];

            spawnEnemy(
                gm, event.type, SCREEN_WIDTH + SPAWN_MARGIN, event.y, hooks);
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
        GameManager &gm, EnemyDirector &, const MasterClock &clock)
    {
        const float delta = elapsedSeconds(clock);

        gm.view<Enemy, Position2D, Hitbox>().each(
            [&](Entity e, Enemy &, Position2D &pos, Hitbox &box) {
                if (leftScreen(rectOf(pos, box), false))
                    destroyNetworked(gm, e);
            });
        gm.view<Projectile, Position2D, Hitbox>().each(
            [&](Entity e, Projectile &, Position2D &pos, Hitbox &box) {
                if (leftScreen(rectOf(pos, box), true))
                    destroyNetworked(gm, e);
            });
        gm.view<Lifetime>().each([&](Entity e, Lifetime &life) {
            life.remaining -= delta;
            if (life.remaining <= 0.f)
                destroyNetworked(gm, e);
        });
    }

    void systemEnemyCombat(
        GameManager &gm, EnemyDirector &director, const EnemyHooks &hooks)
    {
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
                            health.hp = 0;
                        });
                }
                if (consumed)
                    destroyNetworked(gm, bullet);
            });

        gm.view<Enemy, Health, Position2D, Hitbox>().each([&](Entity enemy,
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
                    targetHealth.hp = 0;
                    health.hp = 0;
                    killEnemy(gm, hooks, enemy, ep, eb);
                });
        });
    }

    void registerEnemySystems(GameManager &gm, EnemyHooks hooks)
    {
        const auto h = std::make_shared<const EnemyHooks>(std::move(hooks));

        gm.createSystem<MasterClock>([](GameManager &g, MasterClock &c) {
            g.view<MovementPattern, Velocity2D>().each(
                [&c](Entity, MovementPattern &m, Velocity2D &v) {
                    systemEnemyPattern(m, v, c);
                });
        });
        gm.createSystem<MasterClock>([h](GameManager &g, MasterClock &c) {
            g.view<Enemy, Weapon, Position2D, Hitbox>().each(
                [&g, &c, &h](
                    Entity, Enemy &e, Weapon &w, Position2D &p, Hitbox &b) {
                    systemEnemyWeapon(g, e, w, p, b, c, *h);
                });
        });
        gm.createSystem<MasterClock>([h](GameManager &g, MasterClock &c) {
            g.view<EnemyDirector>().each(
                [&g, &c, &h](Entity, EnemyDirector &d) {
                    systemEnemyDirector(g, d, c, *h);
                });
        });
        gm.createSystem<MasterClock>([](GameManager &g, MasterClock &c) {
            g.view<Walker, Velocity2D, Position2D, Hitbox>().each(
                [&g, &c](Entity,
                    Walker &w,
                    Velocity2D &v,
                    Position2D &p,
                    Hitbox &b) { systemWalkerMovement(g, w, v, p, b, c); });
        });
        gm.createSystem<MasterClock>([h](GameManager &g, MasterClock &c) {
            g.view<Walker, Weapon, Position2D, Hitbox>().each(
                [&g, &c, &h](
                    Entity, Walker &w, Weapon &wp, Position2D &p, Hitbox &b) {
                    systemWalkerWeapon(g, w, wp, p, b, c, *h);
                });
        });
        gm.createSystem<MasterClock>([](GameManager &g, MasterClock &c) {
            g.view<EnemyDirector>().each([&g, &c](Entity, EnemyDirector &d) {
                systemEnemyCleanup(g, d, c);
            });
        });
        gm.createSystem<EnemyDirector>([h](GameManager &g, EnemyDirector &d) {
            systemEnemyCombat(g, d, *h);
        });
    }

} // namespace RType
