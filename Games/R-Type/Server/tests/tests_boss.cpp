/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_boss
*/

#include <criterion/criterion.h>

#include "Common/Background.hpp"
#include "Common/Terrain.hpp"
#include "Enemies/EnemySystems.hpp"
#include "Enemies/EnemyVisuals.hpp"
#include "SpriteSheet2D.hpp"

using namespace RType;

namespace {

    void addBackgrounds(GameManager &gm, float x)
    {
        for (int i = 0; i < 2; ++i) {
            const Entity e = gm.createEntity();

            gm.addComponent<Server::Background>(e);
            gm.addComponent<Position2D>(
                e, x + float(i) * Server::BACKGROUND_PERIOD, 0.f);
            gm.addComponent<Velocity2D>(e, -Server::BACKGROUND_SPEED, 0.f);
        }
    }

    Entity spawnBoss(GameManager &gm)
    {
        return spawnEnemy(gm, EnemyType::Boss, BOSS_DEF.x, BOSS_DEF.y);
    }

    /** @brief Player shot centered on the boss's hitbox. */
    Entity shootBoss(GameManager &gm, Entity boss, int damage)
    {
        const Entity shot = gm.createEntity();
        const auto &pos = gm.getComponent<Position2D>(boss);
        const auto &box = gm.getComponent<Hitbox>(boss);

        gm.addComponent<Projectile>(shot, Projectile {damage, true});
        gm.addComponent<Position2D>(shot,
            pos.x + box.offsetX + box.width / 2.f,
            pos.y + box.offsetY + box.height / 2.f);
        gm.addComponent<Hitbox>(shot, Hitbox {10.f, 10.f});
        return shot;
    }

    int scoutCount(GameManager &gm)
    {
        int count = 0;

        for (auto [_, enemy] : gm.view<Enemy>().getVector())
            count += enemy.type == EnemyType::Scout ? 1 : 0;
        return count;
    }

    float scrollTo(float levelX)
    {
        return SCREEN_WIDTH + SPAWN_MARGIN - levelX * LEVEL_SCALE;
    }

    int bossCount(GameManager &gm)
    {
        int count = 0;

        for (auto [_, boss] : gm.view<Boss>().getVector())
            ++count;
        return count;
    }

} // namespace

Test(boss, level_stops_at_the_boss_room)
{
    GameManager gm;

    addBackgrounds(gm, Server::LEVEL_END_X - 7.f);
    Server::backgroundScroll(gm);
    cr_assert(Server::levelEnded(gm));
    for (auto [_, bg, pos, velocity] :
        gm.view<Server::Background, Position2D, Velocity2D>().getVector())
        cr_assert_float_eq(velocity.x, 0.f, 0.001f);
    Server::resetBackground(gm);
    cr_assert_not(Server::levelEnded(gm));
}

Test(boss, spawns_once_at_level_end)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;

    director.addWave(EnemyType::Scout, 300.f, 1, 0.f, 0.f);
    addBackgrounds(gm, 0.f);
    systemEnemyDirector(gm, director, clock, {});
    cr_assert_eq(bossCount(gm), 0);
    addBackgrounds(gm, Server::LEVEL_END_X);
    systemEnemyDirector(gm, director, clock, {});
    systemEnemyDirector(gm, director, clock, {});
    cr_assert_eq(bossCount(gm), 1);
}

Test(boss, health_scales_with_the_players)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;

    director.players = 3;
    addBackgrounds(gm, Server::LEVEL_END_X);
    systemEnemyDirector(gm, director, clock, {});
    for (auto [_, boss, health] : gm.view<Boss, Health>().getVector()) {
        cr_assert_eq(health.hp, enemyDef(EnemyType::Boss).hp * 3);
        cr_assert_eq(health.maxHp, health.hp);
    }
    cr_assert_eq(bossCount(gm), 1);
}

Test(boss, closed_belly_absorbs_shots)
{
    GameManager gm;
    EnemyDirector director;
    const Entity boss = spawnBoss(gm);
    const Entity shot = shootBoss(gm, boss, 5);
    const int hp = gm.getComponent<Health>(boss).hp;

    gm.getComponent<Boss>(boss).state = BossState::Closed;
    systemEnemyCombat(gm, director, {});
    cr_assert_eq(gm.getComponent<Health>(boss).hp, hp);
    cr_assert_not(gm.hasComponent<Projectile>(shot));
}

Test(boss, open_belly_takes_damage)
{
    GameManager gm;
    EnemyDirector director;
    const Entity boss = spawnBoss(gm);
    const int hp = gm.getComponent<Health>(boss).hp;

    gm.getComponent<Boss>(boss).state = BossState::Open;
    shootBoss(gm, boss, 5);
    systemEnemyCombat(gm, director, {});
    cr_assert_eq(gm.getComponent<Health>(boss).hp, hp - 5);
}

Test(boss, killing_blow_starts_dying_sequence)
{
    GameManager gm;
    EnemyDirector director;
    const Entity boss = spawnBoss(gm);

    gm.getComponent<Boss>(boss).state = BossState::Open;
    gm.getComponent<Health>(boss).hp = 1;
    shootBoss(gm, boss, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert(gm.hasComponent<Boss>(boss));
    cr_assert(gm.getComponent<Boss>(boss).state == BossState::Dying);
    cr_assert_eq(director.score, enemyDef(EnemyType::Boss).scoreValue);
}

Test(boss, contact_kills_player_not_boss)
{
    GameManager gm;
    EnemyDirector director;
    const Entity boss = spawnBoss(gm);
    const Entity player = gm.createEntity();
    const auto &pos = gm.getComponent<Position2D>(boss);
    const auto &box = gm.getComponent<Hitbox>(boss);

    gm.addComponent<EnemyTarget>(player);
    gm.addComponent<Health>(player, Health {1, 1});
    gm.addComponent<Position2D>(
        player, pos.x + box.offsetX + 10.f, pos.y + box.offsetY + 10.f);
    gm.addComponent<Hitbox>(player, Hitbox {20.f, 20.f});
    systemEnemyCombat(gm, director, {});
    cr_assert_eq(gm.getComponent<Health>(player).hp, 0);
    cr_assert(gm.getComponent<Health>(boss).hp > 0);
}

Test(boss, flashes_white_when_hurt)
{
    GameManager gm;
    EnemyDirector director;
    const Entity boss = spawnBoss(gm);
    auto &state = gm.getComponent<Boss>(boss);
    auto &health = gm.getComponent<Health>(boss);

    shootBoss(gm, boss, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert_float_eq(health.hurtTimer, 0.f, 0.001f);
    state.state = BossState::Open;
    shootBoss(gm, boss, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert_float_eq(health.hurtTimer, ENEMY_HURT_FLASH, 0.001f);
}

Test(boss, hurt_flash_blinks_on_the_white_frames)
{
    Health heath;
    Boss boss;
    SpriteSheet2D sheet(
        {BOSS_SHEET.rows, BOSS_SHEET.frames / BOSS_SHEET.rows}, 120, 1.f);

    boss.stage = 2;
    heath.hurtTimer = ENEMY_HURT_FLASH - 0.01f;
    systemBossAnimation(boss, heath, sheet);
    cr_assert_eq(sheet.currentTile,
        BOSS_FLASH_OFFSET + std::size_t(2 * BOSS_IDLE_FRAMES));
    heath.hurtTimer = ENEMY_HURT_BLINK + 0.01f;
    systemBossAnimation(boss, heath, sheet);
    cr_assert_eq(sheet.currentTile, std::size_t(2 * BOSS_IDLE_FRAMES));
    heath.hurtTimer = 0.f;
    systemBossAnimation(boss, heath, sheet);
    cr_assert_eq(sheet.currentTile, std::size_t(2 * BOSS_IDLE_FRAMES));
}

Test(scout, spawns_groups_only_in_open_zones)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;

    addBackgrounds(gm, scrollTo(1000.f));
    systemEnemyDirector(gm, director, clock, {});
    cr_assert_geq(scoutCount(gm), SCOUT_DEF.minGroup);
    cr_assert_leq(scoutCount(gm), SCOUT_DEF.maxGroup);
    systemEnemyDirector(gm, director, clock, {});
    cr_assert_leq(scoutCount(gm), SCOUT_DEF.maxGroup);

    GameManager middle;
    EnemyDirector other;

    addBackgrounds(middle, scrollTo(2400.f));
    systemEnemyDirector(middle, other, clock, {});
    cr_assert_eq(scoutCount(middle), 0);
}

Test(scout, never_spawns_into_walls)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;
    Server::Terrain terrain;

    terrain.mask = {.width = Server::BACKGROUND_WIDTH,
        .height = 240,
        .solid = std::vector<bool>(
            std::size_t(Server::BACKGROUND_WIDTH) * 240, true)};
    gm.addComponent<Server::Terrain>(gm.createEntity(), std::move(terrain));
    addBackgrounds(gm, scrollTo(1000.f));
    systemEnemyDirector(gm, director, clock, {});
    cr_assert_eq(scoutCount(gm), 0);
}

Test(scout, group_members_move_differently)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;
    std::vector<MovementPattern> motions;

    addBackgrounds(gm, scrollTo(1000.f));
    systemEnemyDirector(gm, director, clock, {});
    for (auto [_, enemy, motion] :
        gm.view<Enemy, MovementPattern>().getVector()) {
        cr_assert(motion.type == PatternType::ZigZag);
        for (const auto &other : motions)
            cr_assert(motion.amplitude != other.amplitude ||
                motion.period != other.period ||
                motion.horizontalVelocity != other.horizontalVelocity);
        motions.push_back(motion);
    }
    cr_assert_geq(motions.size(), std::size_t(SCOUT_DEF.minGroup));
}

Test(scout, zigzags)
{
    GameManager gm;
    const Entity scout = spawnEnemy(gm, EnemyType::Scout, 1000.f, 500.f);

    cr_assert(gm.getComponent<MovementPattern>(scout).type ==
        PatternType::ZigZag);
}
