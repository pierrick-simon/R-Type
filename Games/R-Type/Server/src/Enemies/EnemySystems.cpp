/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy movement systems
*/

#include "Enemies/EnemySystems.hpp"
#include "Background.hpp"
#include "Config.hpp"
#include "Destroy.hpp"
#include "Terrain.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

namespace RType {

    static_assert(LEVEL_SCALE == Server::SCALE,
        "the boss is placed on the level's wall, both must match");

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
            case PatternType::AlienBeam:
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

        bool bossShielded(GameManager &gm, Entity enemy)
        {
            return gm.hasComponent<Boss>(enemy) &&
                !bossExposed(gm.getComponent<Boss>(enemy));
        }

        void killEnemy(GameManager &gm, const EnemyHooks &hooks, Entity enemy,
            const Position2D &pos, const Hitbox &box)
        {
            if (gm.hasComponent<Boss>(enemy)) {
                // Explodes for a while before vanishing, see systemBoss.
                auto &boss = gm.getComponent<Boss>(enemy);

                boss.state = BossState::Dying;
                boss.timer = 0.f;
                return;
            }
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

    namespace {

        constexpr float ALIEN_HEIGHT = float(ALIEN_SHEET.cellH) * SPRITE_SCALE;

        void setAlienState(Alien &alien, AlienState state)
        {
            alien.state = state;
            alien.timer = 0.f;
        }

        /** @brief Alive target whose center is the closest to height y. */
        bool findClosestTarget(GameManager &gm, float y, float &tx, float &ty)
        {
            bool found = false;

            gm.view<EnemyTarget, Health, Position2D, Hitbox>().each(
                [&](Entity,
                    EnemyTarget &,
                    Health &health,
                    Position2D &pos,
                    Hitbox &box) {
                    const float cy = pos.y + box.offsetY + box.height / 2.f;

                    if (health.hp <= 0 ||
                        (found && std::fabs(cy - y) >= std::fabs(ty - y)))
                        return;
                    tx = pos.x + box.offsetX + box.width / 2.f;
                    ty = cy;
                    found = true;
                });
            return found;
        }

        float floorUnder(GameManager &gm, float x)
        {
            const auto platform = Server::platformAt(gm, x);

            return platform ? platform->top : SCREEN_HEIGHT;
        }

        /** @brief Heads to (dx, dy) away without overshooting it. */
        void moveToward(
            Velocity2D &velocity, float dx, float dy, float speed, float delta)
        {
            const float length = std::sqrt(dx * dx + dy * dy);

            if (length < 1.f || delta <= 0.f) {
                velocity = {0.f, 0.f};
                return;
            }
            const float v = std::min(speed, length / delta);
            velocity = {dx / length * v, dy / length * v};
        }

        /**
         * @brief Gets in line with the closest player, then starts charging.
         * @return true once the alien starts charging.
         */
        bool alienAim(GameManager &gm, Alien &alien, Velocity2D &velocity,
            const Position2D &pos, const Hitbox &hitbox, float delta)
        {
            const float x = centerX(pos, hitbox);
            float targetX = 0.f;
            float targetY = 0.f;
            const bool found = findClosestTarget(
                gm, alienMuzzle(alien, pos).second, targetX, targetY);

            if (found)
                alien.facingLeft = targetX < x;
            const float gap =
                found ? targetY - alienMuzzle(alien, pos).second : 0.f;

            if (alien.grounded) {
                velocity = {FLOOR_VELOCITY, 0.f};
                return found && std::fabs(gap) <= ALIEN_DEF.alignTolerance;
            }
            const float goalY = std::clamp(
                pos.y + gap, ALIEN_DEF.minY, floorUnder(gm, x) - ALIEN_HEIGHT);

            moveToward(velocity,
                ALIEN_DEF.hoverX - x,
                goalY - pos.y,
                enemyDef(EnemyType::Alien).speed,
                delta);
            return found && x < SCREEN_WIDTH - hitbox.width &&
                std::fabs(gap) <= ALIEN_DEF.alignTolerance;
        }

        void alienAfterShot(Alien &alien)
        {
            if (alien.grounded)
                setAlienState(alien, AlienState::TakeOff);
            else if (alien.shots >= ALIEN_DEF.maxShots)
                setAlienState(alien, AlienState::Exit);
            else if (alien.shots % ALIEN_DEF.shotsBeforeLanding == 0)
                setAlienState(alien, AlienState::Descend);
            else
                setAlienState(alien, AlienState::Fly);
        }

    } // namespace

    void systemAlien(GameManager &gm, Entity entity, Alien &alien,
        Velocity2D &velocity, Position2D &pos, const Hitbox &hitbox,
        const MasterClock &clock, const EnemyHooks &hooks)
    {
        const float delta = elapsedSeconds(clock);
        const Velocity2D still = {alien.grounded ? FLOOR_VELOCITY : 0.f, 0.f};

        alien.timer += delta;
        switch (alien.state) {
            case AlienState::Fly:
            case AlienState::Landed:
                if (alienAim(gm, alien, velocity, pos, hitbox, delta) &&
                    alien.timer >= ALIEN_DEF.cooldown) {
                    velocity = still;
                    setAlienState(alien, AlienState::Charge);
                    if (hooks.onAlienCharge)
                        hooks.onAlienCharge(gm, entity, ALIEN_DEF.chargeTime);
                } else if (alien.state == AlienState::Landed &&
                    alien.timer >= ALIEN_DEF.landedTime)
                    setAlienState(alien, AlienState::TakeOff);
                if (alien.grounded)
                    pos.y = floorUnder(gm, centerX(pos, hitbox)) - ALIEN_HEIGHT;
                break;
            case AlienState::Charge:
                velocity = still;
                if (alien.timer >= ALIEN_DEF.chargeTime) {
                    const auto [x, y] = alienMuzzle(alien, pos);

                    fireAlienBeam(gm, x, y, alien.facingLeft, hooks);
                    ++alien.shots;
                    setAlienState(alien, AlienState::Cooldown);
                }
                break;
            case AlienState::Cooldown:
                velocity = still;
                if (alien.timer >= ALIEN_DEF.cooldown)
                    alienAfterShot(alien);
                break;
            case AlienState::Descend: {
                const float floor = floorUnder(gm, centerX(pos, hitbox));

                velocity = {0.f, ALIEN_DEF.landSpeed};
                if (pos.y + ALIEN_HEIGHT >= floor) {
                    pos.y = floor - ALIEN_HEIGHT;
                    velocity = {FLOOR_VELOCITY, 0.f};
                    alien.grounded = true;
                    setAlienState(alien, AlienState::Landed);
                }
                break;
            }
            case AlienState::TakeOff:
                alien.grounded = false;
                velocity = {0.f, -ALIEN_DEF.landSpeed};
                if (alien.timer >= ALIEN_DEF.takeOffTime)
                    setAlienState(alien,
                        alien.shots >= ALIEN_DEF.maxShots ? AlienState::Exit
                                                          : AlienState::Fly);
                break;
            case AlienState::Exit:
                alien.facingLeft = true;
                velocity = {-1.5f * enemyDef(EnemyType::Alien).speed,
                    -0.3f * enemyDef(EnemyType::Alien).speed};
                break;
        }
    }

    namespace {

        void setBossState(Boss &boss, BossState state)
        {
            boss.state = state;
            boss.timer = 0.f;
        }

        void bossVolley(
            GameManager &gm, const Position2D &pos, const EnemyHooks &hooks)
        {
            const auto [x, y] = bossMuzzle(pos);
            float targetX = 0.f;
            float targetY = 0.f;

            if (!findClosestTarget(gm, y, targetX, targetY))
                return;
            const float aim = std::atan2(targetY - y, targetX - x);

            for (int i = 0; i < BOSS_DEF.burst; ++i) {
                const float angle =
                    aim + BOSS_DEF.spread * float(i - BOSS_DEF.burst / 2);

                fireEnemyBullet(gm,
                    x,
                    y,
                    std::cos(angle) * BOSS_DEF.bulletSpeed,
                    std::sin(angle) * BOSS_DEF.bulletSpeed,
                    BOSS_DEF.bulletDamage,
                    hooks,
                    BOSS_BULLET_SHEET);
            }
        }

        void bossDying(GameManager &gm, Entity entity, Boss &boss,
            const Position2D &pos, const EnemyHooks &hooks)
        {
            while (boss.timer < BOSS_DEF.deathTime &&
                boss.timer >=
                    float(boss.explosions) * BOSS_DEF.explosionInterval) {
                // R2 low-discrepancy sequence: blasts spread over the body.
                const float n = float(++boss.explosions);
                const float u = std::fmod(n * 0.7548777f, 1.f);
                const float v = std::fmod(n * 0.5698403f, 1.f);

                if (hooks.onBossExplosion)
                    hooks.onBossExplosion(gm,
                        pos.x + BOSS_WIDTH * (0.2f + 0.7f * u),
                        pos.y + BOSS_HEIGHT * (0.1f + 0.8f * v));
            }
            if (boss.timer < BOSS_DEF.deathTime)
                return;
            for (auto [_, director] : gm.view<EnemyDirector>().getVector())
                director.victoryTimer = BOSS_DEF.victoryDelay;
            destroyNetworked(gm, entity);
        }

    } // namespace

    void systemBoss(GameManager &gm, Entity entity, Boss &boss,
        Velocity2D &velocity, Position2D &pos, const MasterClock &clock,
        const EnemyHooks &hooks)
    {
        const float delta = elapsedSeconds(clock);
        const float stagesTime = BOSS_DEF.stageTime * float(BOSS_STAGES - 1);

        boss.timer += delta;
        velocity = {0.f, 0.f};
        switch (boss.state) {
            case BossState::Enter:
                velocity.x = -BOSS_DEF.enterSpeed;
                if (pos.x <= BOSS_DEF.x) {
                    pos.x = BOSS_DEF.x;
                    velocity.x = 0.f;
                    setBossState(boss, BossState::Closed);
                }
                break;
            case BossState::Closed:
                if (boss.timer >= BOSS_DEF.closedTime)
                    setBossState(boss, BossState::Opening);
                break;
            case BossState::Opening:
                boss.stage = std::min(
                    BOSS_STAGES - 1, int(boss.timer / BOSS_DEF.stageTime));
                if (boss.timer >= stagesTime) {
                    boss.fireTimer = 0.f;
                    setBossState(boss, BossState::Open);
                }
                break;
            case BossState::Open:
                boss.fireTimer += delta;
                if (boss.fireTimer >= BOSS_DEF.fireCooldown) {
                    boss.fireTimer = 0.f;
                    bossVolley(gm, pos, hooks);
                }
                if (boss.timer >= BOSS_DEF.openTime)
                    setBossState(boss, BossState::Closing);
                break;
            case BossState::Closing:
                boss.stage = std::max(
                    0, BOSS_STAGES - 1 - int(boss.timer / BOSS_DEF.stageTime));
                if (boss.timer >= stagesTime)
                    setBossState(boss, BossState::Closed);
                break;
            case BossState::Dying:
                bossDying(gm, entity, boss, pos, hooks);
                break;
        }
    }

    void systemEnemyDirector(GameManager &gm, EnemyDirector &director,
        const MasterClock &clock, const EnemyHooks &hooks)
    {
        const float delta = elapsedSeconds(clock);

        if (director.victoryTimer > 0.f) {
            director.victoryTimer -= delta;
            if (director.victoryTimer <= 0.f)
                director.levelCleared = true;
        }
        if (Server::levelEnded(gm)) {
            if (!director.bossSpawned)
                spawnEnemy(
                    gm, EnemyType::Boss, SCREEN_WIDTH, BOSS_DEF.y, hooks);
            director.bossSpawned = true;
            return;
        }
        if (director.timeline.empty())
            return;
        director.elapsed += delta;
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
                            if (bossShielded(gm, enemy)) {
                                consumed = true;
                                return;
                            }
                            const int hp = health.hp;
                            health.hp -= shot.damage;
                            shot.damage -= hp;
                            consumed = !shot.piercing || shot.damage <= 0;
                            if (health.hp > 0)
                                return;
                            director.score += info.scoreValue;
                            killEnemy(gm, hooks, enemy, ep, eb);
                        });
                } else {
                    gm.view<EnemyTarget, Health, Position2D, Hitbox>().each(
                        [&](Entity target,
                            EnemyTarget &,
                            Health &health,
                            Position2D &tp,
                            Hitbox &tb) {
                            if (consumed || health.hp <= 0 ||
                                gm.hasComponent<Invincible>(target) ||
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
                [&](Entity target,
                    EnemyTarget &,
                    Health &targetHealth,
                    Position2D &tp,
                    Hitbox &tb) {
                    if (health.hp <= 0 || targetHealth.hp <= 0 ||
                        gm.hasComponent<Invincible>(target) ||
                        !overlap(enemyRect, rectOf(tp, tb)))
                        return;
                    targetHealth.hp = 0;
                    if (gm.hasComponent<Boss>(enemy))
                        return;
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
        gm.createSystem<MasterClock>([h](GameManager &g, MasterClock &c) {
            g.view<Alien, Velocity2D, Position2D, Hitbox>().each(
                [&g, &c, &h](Entity e,
                    Alien &a,
                    Velocity2D &v,
                    Position2D &p,
                    Hitbox &b) { systemAlien(g, e, a, v, p, b, c, *h); });
        });
        gm.createSystem<MasterClock>([h](GameManager &g, MasterClock &c) {
            g.view<Boss, Velocity2D, Position2D>().each(
                [&g, &c, &h](Entity e, Boss &b, Velocity2D &v, Position2D &p) {
                    systemBoss(g, e, b, v, p, c, *h);
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
