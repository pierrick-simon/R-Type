/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_lives
*/

#include <chrono>
#include <criterion/criterion.h>
#include <thread>

#include "Components.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Enemies/EnemyManager.hpp"
#include "GameManager.hpp"
#include "GameState.hpp"
#include "Player/Player.hpp"
#include "SpriteSheet2D.hpp"

using namespace RType;
using namespace RType::Server;

namespace {

    Entity addPlayer(GameManager &gm)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Player>(e);
        gm.addComponent<Health>(e, Health {PLAYER_HP, PLAYER_HP});
        gm.addComponent<Position2D>(e, 500.f, 500.f);
        gm.addComponent<Velocity2D>(e);
        gm.addComponent<SpriteSheet2D>(e);
        return e;
    }

    /** @brief Kills the player and lets its explosion play out. */
    void die(GameManager &gm, MasterClock &clock, Entity player)
    {
        gm.getComponent<Health>(player).hp = 0;
        playerDeath(gm, clock);
        std::this_thread::sleep_for(
            std::chrono::duration<float>(PLAYER_DEATH_DURATION + 0.05f));
        MasterClock::systemMasterClock(clock);
        playerDeath(gm, clock);
        systemGameState(gm);
    }

} // namespace

Test(lives, death_costs_a_life_and_respawns_alone)
{
    GameManager gm;
    MasterClock clock;
    const Entity a = addPlayer(gm);
    const Entity b = addPlayer(gm);

    gm.getComponent<Position2D>(b) = {800.f, 300.f};
    die(gm, clock, a);
    cr_assert_eq(gm.getComponent<Player>(a).lives, PLAYER_LIVES - 1);
    cr_assert_eq(gm.getComponent<Health>(a).hp, PLAYER_HP);
    cr_assert_not(gm.getComponent<Player>(a).dying);
    cr_assert_eq(gm.getComponent<Player>(b).lives, PLAYER_LIVES);
    cr_assert_float_eq(gm.getComponent<Position2D>(b).x, 800.f, 0.01f);
}

Test(lives, out_of_lives_waits_for_the_team)
{
    GameManager gm;
    MasterClock clock;
    const Entity a = addPlayer(gm);
    const Entity b = addPlayer(gm);

    for (int i = 0; i < PLAYER_LIVES; ++i)
        die(gm, clock, a);
    cr_assert_eq(gm.getComponent<Player>(a).lives, 0);
    cr_assert_eq(gm.getComponent<Health>(a).hp, 0);
    cr_assert_eq(gm.getComponent<Player>(b).lives, PLAYER_LIVES);
    cr_assert_eq(gm.getComponent<Health>(b).hp, PLAYER_HP);
}

Test(lives, game_over_waits_then_restarts)
{
    GameManager gm;
    MasterClock clock;
    const Entity a = addPlayer(gm);
    const Entity b = addPlayer(gm);
    const Entity director = gm.createEntity();

    gm.addComponent<EnemyDirector>(director);
    gm.getComponent<EnemyDirector>(director).score = 700;
    for (int i = 0; i < PLAYER_LIVES; ++i)
        die(gm, clock, a);
    cr_assert_not(isGameOverScreen());
    for (int i = 0; i < PLAYER_LIVES; ++i)
        die(gm, clock, b);
    cr_assert(isGameOverScreen());
    cr_assert(gm.getComponent<EnemyDirector>(director).paused);
    cr_assert_eq(gm.getComponent<EnemyDirector>(director).score, 700);
    cr_assert_eq(gm.getComponent<Player>(a).lives, 0);
    restartGame(gm);
    cr_assert_not(isGameOverScreen());
    cr_assert_not(gm.getComponent<EnemyDirector>(director).paused);
    for (const Entity e : {a, b}) {
        cr_assert_eq(gm.getComponent<Player>(e).lives, PLAYER_LIVES);
        cr_assert_eq(gm.getComponent<Health>(e).hp, PLAYER_HP);
        cr_assert_not(gm.getComponent<Player>(e).dying);
    }
}

Test(lives, respawn_grants_invincibility)
{
    GameManager gm;
    MasterClock clock;
    const Entity a = addPlayer(gm);

    cr_assert_not(gm.hasComponent<Invincible>(a));
    die(gm, clock, a);
    cr_assert(gm.hasComponent<Invincible>(a));
}

Test(lives, invincibility_wears_off)
{
    GameManager gm;
    MasterClock clock;
    const Entity a = addPlayer(gm);

    gm.addComponent<Invincible>(a, Invincible {0.01f});
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    MasterClock::systemMasterClock(clock);
    playerInvincibility(gm, clock);
    cr_assert_not(gm.hasComponent<Invincible>(a));
    cr_assert_eq(gm.getComponent<SpriteSheet2D>(a).color.a, 255);
}
