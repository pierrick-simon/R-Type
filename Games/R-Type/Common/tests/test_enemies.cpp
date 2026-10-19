/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** test_enemies
*/

#include <criterion/criterion.h>

#include <chrono>
#include <cmath>
#include <set>
#include <string>
#include <thread>

#include "EnemyDefs.hpp"
#include "EnemyFactory.hpp"
#include "EnemySystems.hpp"
#include "Position2D.hpp"

using namespace RType;
using namespace RType::Game;

namespace {
    /// Same setup as the real game: engine systems first, then the enemies.
    void setupGame(GameManager &gm)
    {
        gm.createSystem<Clock>(Clock::systemClock);
        gm.createSystem<Velocity2D, Position2D, Clock>(
            Velocity2D::systemVelocity2D);
        registerEnemySystems(gm);
    }

    void runFrames(GameManager &gm, int frames)
    {
        for (int i = 0; i < frames; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            gm.update();
        }
    }
} // namespace

Test(enemy_defs, table_matches_enum_order)
{
    const char *expected[] = {"scout",
        "spike_orb",
        "armored_eye",
        "pod_spinner",
        "twin_ship",
        "walker",
        "gunship_cyan",
        "gunship_pink",
        "gunship_green",
        "gunship_red",
        "gunship_blue"};

    cr_assert_eq(std::size(expected), std::size_t(EnemyType::Count));
    for (std::size_t i = 0; i < std::size(expected); i++)
        cr_assert_str_eq(enemyDef(EnemyType(i)).name, expected[i]);
}

Test(enemy_defs, every_enemy_is_playable)
{
    std::set<std::string> sprites;

    for (const EnemyDef &def : ENEMY_DEFS) {
        cr_assert_not_null(def.sheet, "%s has no sprite", def.name);
        cr_assert_gt(def.hp, 0, "%s", def.name);
        cr_assert_gt(def.speed, 0.f, "%s", def.name);
        cr_assert_gt(def.sheet->frames, 0, "%s", def.name);
        cr_assert_gt(def.sheet->hitW, 0, "%s", def.name);
        cr_assert_gt(def.sheet->hitH, 0, "%s", def.name);
        cr_assert_gt(def.period, 0.f, "%s", def.name);
        sprites.insert(def.sheet->file);
    }
    cr_assert_eq(
        sprites.size(), ENEMY_DEFS.size(), "two enemies share a sprite");
}

Test(enemy_pattern, straight_has_no_vertical_speed)
{
    MovementPattern p {PatternType::Straight, 100.f, 50.f, 2.f, 0.7f};

    cr_assert_float_eq(p.velocityY(), 0.f, 1e-6f);
}

Test(enemy_pattern, sine_starts_going_down_and_peaks_at_start)
{
    // y = A sin(w t): the speed is maximal at t = 0 and null at a quarter.
    MovementPattern p {PatternType::SineWave, 100.f, 100.f, 2.f, 0.f};
    float w = 2.f * std::numbers::pi_v<float> / 2.f;

    cr_assert_float_eq(p.velocityY(), 100.f * w, 1e-3f);
    p.time = 0.5f;
    cr_assert_float_eq(p.velocityY(), 0.f, 1e-3f);
    p.time = 1.f;
    cr_assert_float_eq(p.velocityY(), -100.f * w, 1e-3f);
}

// Test(enemy_pattern, zigzag_alternates_and_stays_centered)
// {
//     MovementPattern p {PatternType::ZigZag, 100.f, 50.f, 2.f, 0.f};
//     float slope = 4.f * 50.f / 2.f;

//     cr_assert_float_eq(p.velocityY(), slope, 1e-4f);
//     p.time = 0.6f; // past the first quarter: goes back up
//     cr_assert_float_eq(p.velocityY(), -slope, 1e-4f);
//     p.time = 1.6f; // past the third quarter: goes down again
//     cr_assert_float_eq(p.velocityY(), slope, 1e-4f);

//     // Over a whole period the net displacement is null (stays centered).
//     float y = 0.f;
//     float dt = 0.001f;
//     for (p.time = 0.f; p.time < 2.f; p.time += dt)
//         y += p.velocityY() * dt;
//     cr_assert_float_eq(y, 0.f, 0.5f);
// }

Test(enemy_factory, spawn_sets_all_the_components)
{
    GameManager gm;
    Entity e = EnemyFactory::spawn(gm, EnemyType::SpikeOrb, 800.f, 300.f);
    const EnemyDef &def = enemyDef(EnemyType::SpikeOrb);

    cr_assert(gm.hasEntity(e));
    cr_assert_float_eq(gm.getComponent<Position2D>(e).x, 800.f, 1e-6f);
    cr_assert_float_eq(gm.getComponent<Position2D>(e).y, 300.f, 1e-6f);
    cr_assert_eq(gm.getComponent<Enemy>(e).type, EnemyType::SpikeOrb);
    cr_assert_eq(gm.getComponent<Health>(e).hp, def.hp);
    cr_assert_eq(gm.getComponent<Health>(e).maxHp, def.hp);
    cr_assert_gt(gm.getComponent<Hitbox>(e).width, 0.f);
    cr_assert_float_eq(gm.getComponent<Velocity2D>(e).x, -def.speed, 1e-6f);
    cr_assert(gm.hasComponent<Clock>(e));
    cr_assert(gm.hasComponent<MovementPattern>(e));
}

Test(enemy_factory, every_type_can_be_spawned)
{
    GameManager gm;

    for (std::size_t i = 0; i < std::size_t(EnemyType::Count); i++) {
        Entity e = EnemyFactory::spawn(gm, EnemyType(i), 0.f, 0.f);
        cr_assert_eq(gm.getComponent<Enemy>(e).type, EnemyType(i));
    }
}

Test(enemy_systems, enemies_move_to_the_left)
{
    GameManager gm;
    setupGame(gm);
    Entity e = EnemyFactory::spawn(gm, EnemyType::Scout, 1000.f, 300.f);

    runFrames(gm, 5);
    cr_assert_lt(gm.getComponent<Position2D>(e).x, 1000.f);
    cr_assert_float_eq(gm.getComponent<Position2D>(e).y, 300.f, 1e-3f);
}

Test(enemy_systems, sine_enemy_leaves_its_height)
{
    GameManager gm;
    setupGame(gm);
    Entity e = EnemyFactory::spawn(gm, EnemyType::SpikeOrb, 1000.f, 300.f);

    runFrames(gm, 10);
    cr_assert_neq(gm.getComponent<Position2D>(e).y, 300.f);
}

Test(enemy_systems, dead_enemy_is_destroyed)
{
    GameManager gm;
    setupGame(gm);
    Entity e = EnemyFactory::spawn(gm, EnemyType::PodSpinner, 1000.f, 300.f);

    gm.getComponent<Health>(e).hp = 0;
    gm.update();
    cr_assert_not(gm.hasEntity(e));
}

Test(enemy_systems, enemy_out_of_screen_is_destroyed)
{
    GameManager gm;
    setupGame(gm);
    Entity gone = EnemyFactory::spawn(gm, EnemyType::Scout, -500.f, 300.f);
    Entity here = EnemyFactory::spawn(gm, EnemyType::Scout, 500.f, 300.f);

    gm.update();
    cr_assert_not(gm.hasEntity(gone));
    cr_assert(gm.hasEntity(here));
}
