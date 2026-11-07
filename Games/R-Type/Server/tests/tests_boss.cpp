/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_boss
*/

#include <criterion/criterion.h>

#include "Common/Background.hpp"
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

    shootBoss(gm, boss, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert_float_eq(state.hurtTimer, 0.f, 0.001f);
    state.state = BossState::Open;
    shootBoss(gm, boss, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert_float_eq(state.hurtTimer, BOSS_HURT_FLASH, 0.001f);
}

Test(boss, hurt_flash_blinks_on_the_white_frames)
{
    Boss boss;
    SpriteSheet2D sheet(
        {BOSS_SHEET.rows, BOSS_SHEET.frames / BOSS_SHEET.rows}, 120, 1.f);

    boss.stage = 2;
    boss.hurtTimer = BOSS_HURT_FLASH - 0.01f;
    systemBossAnimation(boss, sheet);
    cr_assert_eq(sheet.currentTile,
        BOSS_FLASH_OFFSET + std::size_t(2 * BOSS_IDLE_FRAMES));
    boss.hurtTimer = BOSS_HURT_BLINK + 0.01f;
    systemBossAnimation(boss, sheet);
    cr_assert_eq(sheet.currentTile, std::size_t(2 * BOSS_IDLE_FRAMES));
    boss.hurtTimer = 0.f;
    systemBossAnimation(boss, sheet);
    cr_assert_eq(sheet.currentTile, std::size_t(2 * BOSS_IDLE_FRAMES));
}
