/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_weapons
*/

#include <chrono>
#include <criterion/criterion.h>
#include <thread>

#include "Common/Bullet.hpp"
#include "Common/Terrain.hpp"
#include "Components.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemySystems.hpp"
#include "SpriteSheet2D.hpp"
#include "Weapons/Force.hpp"
#include "Weapons/Laser.hpp"
#include "Weapons/PowerUp.hpp"

using namespace RType;
using namespace RType::Server;

namespace {

    Entity addPlayer(GameManager &gm)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Player>(e);
        gm.addComponent<Health>(e, Health {PLAYER_HP, PLAYER_HP});
        gm.addComponent<Hitbox>(e, Hitbox {PLAYER_WIDTH, PLAYER_HEIGHT});
        gm.addComponent<Position2D>(e, 500.f, 500.f);
        gm.addComponent<Velocity2D>(e);
        gm.addComponent<EnemyTarget>(e);
        return e;
    }

    /** @brief Drops an item of the given laser right on the player. */
    void pickUp(GameManager &gm, Entity player, LaserType laser)
    {
        const auto &pos = gm.getComponent<Position2D>(player);

        spawnPowerUp(
            gm, laser, pos.x + PLAYER_WIDTH / 2.f, pos.y + PLAYER_HEIGHT / 2.f);
        powerUpPickupSystem(gm);
    }

    int countOf(GameManager &gm, auto tag)
    {
        int count = 0;

        for (auto [_, c] : gm.view<decltype(tag)>().getVector())
            ++count;
        return count;
    }

    Entity forceCenter(GameManager &gm, Entity force, float &x, float &y)
    {
        const auto &pos = gm.getComponent<Position2D>(force);
        const auto &box = gm.getComponent<Hitbox>(force);

        x = pos.x + box.offsetX + box.width / 2.f;
        y = pos.y + box.offsetY + box.height / 2.f;
        return force;
    }

    /** @brief Level with a ceiling (screen y < 45) and a wall (screen x from
     * 900 to 945). */
    void addTerrain(GameManager &gm)
    {
        const int width = int(WINDOW_WIDTH / SCALE);
        const int height = int(WINDOW_HEIGHT / SCALE);
        CollisionMask mask {width, height, {}};
        const Entity terrain = gm.createEntity();
        const Entity background = gm.createEntity();

        mask.solid.resize(std::size_t(width) * std::size_t(height));
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
                mask.solid[std::size_t(y) * std::size_t(width) +
                    std::size_t(x)] = y < 10 || (x >= 200 && x < 210);
        gm.addComponent<Terrain>(terrain, Terrain {std::move(mask), {}});
        gm.addComponent<Background>(background);
        gm.addComponent<Position2D>(background, 0.f, 0.f);
    }

    Entity addReflect(GameManager &gm, float x, float y, Velocity2D velocity)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<ReflectShot>(e);
        gm.addComponent<Position2D>(e, x, y);
        gm.addComponent<Velocity2D>(e, velocity);
        gm.addComponent<SpriteSheet2D>(e,
            Size2D(1, REFLECT_FRAMES),
            REFLECT_STRAIGHT,
            1,
            LASER_SCALE,
            false);
        return e;
    }

    Entity addShot(GameManager &gm, float x, float y, bool fromPlayer)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Projectile>(e, Projectile {1, fromPlayer});
        gm.addComponent<Position2D>(e, x, y);
        gm.addComponent<Hitbox>(e, Hitbox {20.f, 20.f});
        return e;
    }

    void tick(MasterClock &clock)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        MasterClock::systemMasterClock(clock);
    }

} // namespace

Test(weapons, pickup_gives_then_upgrades_the_force)
{
    GameManager gm;
    const Entity player = addPlayer(gm);

    pickUp(gm, player, LaserType::Reflect);
    cr_assert_eq(countOf(gm, PowerUp {}), 0);
    const auto force = findForce(gm, player);

    cr_assert(force.has_value());
    cr_assert_eq(gm.getComponent<Force>(*force).level, 1);
    pickUp(gm, player, LaserType::Helix);
    pickUp(gm, player, LaserType::Ring);
    cr_assert_eq(countOf(gm, Force {player}), 1);
    cr_assert_eq(gm.getComponent<Force>(*force).level, FORCE_MAX_LEVEL);
    cr_assert(gm.getComponent<Force>(*force).laser == LaserType::Ring);
}

Test(weapons, dying_loses_the_force)
{
    GameManager gm;
    const Entity player = addPlayer(gm);

    pickUp(gm, player, LaserType::Reflect);
    gm.getComponent<Player>(player).dying = true;
    forceSystem(gm);
    cr_assert_not(findForce(gm, player).has_value());
}

Test(weapons, force_blocks_enemy_shots)
{
    GameManager gm;
    EnemyDirector director;
    const Entity player = addPlayer(gm);
    float x = 0.f;
    float y = 0.f;

    pickUp(gm, player, LaserType::Reflect);
    forceCenter(gm, *findForce(gm, player), x, y);
    fireEnemyBullet(gm, x, y, -100.f, 0.f, 1);
    systemEnemyCombat(gm, director, {});
    cr_assert_eq(countOf(gm, Projectile {}), 0);
    cr_assert_eq(gm.getComponent<Health>(player).hp, PLAYER_HP);
}

Test(weapons, force_kills_on_contact_and_enemies_drop_loot)
{
    GameManager gm;
    EnemyDirector director;
    EnemyHooks hooks;
    const Entity player = addPlayer(gm);
    int loot = 0;
    float x = 0.f;
    float y = 0.f;

    hooks.onEnemyLoot = [&loot](GameManager &, float, float) {
        ++loot;
    };
    pickUp(gm, player, LaserType::Reflect);
    forceCenter(gm, *findForce(gm, player), x, y);
    spawnEnemy(gm, EnemyType::Scout, x, y);
    systemEnemyCombat(gm, director, hooks);
    cr_assert_eq(countOf(gm, Enemy {}), 0);
    cr_assert_eq(loot, 1);
    cr_assert_eq(gm.getComponent<Health>(player).hp, PLAYER_HP);
}

Test(weapons, reflect_laser_bounces_on_the_ceiling)
{
    GameManager gm;
    MasterClock clock;
    const Entity player = addPlayer(gm);
    bool bounced = false;

    pickUp(gm, player, LaserType::Reflect);
    fireLaser(gm, *findForce(gm, player));
    cr_assert_eq(countOf(gm, ReflectShot {}), 3);
    for (auto [_, shot, pos, velocity, sheet] :
        gm.view<ReflectShot, Position2D, Velocity2D, SpriteSheet2D>()
            .getVector()) {
        if (velocity.y >= 0.f)
            continue;
        pos.y = -1.f;
        reflectSystem(gm, clock);
        cr_assert_gt(velocity.y, 0.f);
        cr_assert_eq(sheet.currentTile, REFLECT_TIP_TOP);
        bounced = true;
    }
    cr_assert(bounced);
}

Test(weapons, helix_is_a_single_piercing_shot)
{
    GameManager gm;
    const Entity player = addPlayer(gm);

    pickUp(gm, player, LaserType::Helix);
    fireLaser(gm, *findForce(gm, player));
    cr_assert_eq(countOf(gm, Projectile {}), 1);
    for (auto [_, shot, sheet] :
        gm.view<Projectile, SpriteSheet2D>().getVector()) {
        cr_assert(shot.piercing);
        cr_assert_eq(sheet.recNb.height, 1);
        cr_assert_eq(sheet.recNb.width, HELIX_FRAMES);
    }
}

Test(weapons, ring_level_two_fires_two_rings)
{
    GameManager gm;
    const Entity player = addPlayer(gm);

    pickUp(gm, player, LaserType::Ring);
    pickUp(gm, player, LaserType::Ring);
    fireLaser(gm, *findForce(gm, player));
    cr_assert_eq(countOf(gm, RingShot {}), 2);
}

Test(weapons, reflect_laser_bounces_off_the_walls)
{
    GameManager gm;
    MasterClock clock;

    addTerrain(gm);
    const Entity straight =
        addReflect(gm, 900.f - REFLECT_WIDTH + 5.f, 500.f, {LASER_SPEED, 0.f});
    const Entity diagonal = addReflect(gm, 200.f, 40.f, {1000.f, -1000.f});

    tick(clock);
    reflectSystem(gm, clock);
    cr_assert_lt(gm.getComponent<Velocity2D>(straight).x, 0.f);
    cr_assert_eq(gm.getComponent<SpriteSheet2D>(straight).currentTile,
        REFLECT_TIP_RIGHT);
    cr_assert_gt(gm.getComponent<Velocity2D>(diagonal).y, 0.f);
    cr_assert_gt(gm.getComponent<Velocity2D>(diagonal).x, 0.f);
    cr_assert_eq(
        gm.getComponent<SpriteSheet2D>(diagonal).currentTile, REFLECT_TIP_TOP);
}

Test(weapons, shots_stop_on_the_walls)
{
    GameManager gm;

    addTerrain(gm);
    const Entity player = addShot(gm, 910.f, 500.f, true);
    const Entity enemy = addShot(gm, 910.f, 500.f, false);

    shotTerrainSystem(gm);
    cr_assert_not(gm.hasComponent<Projectile>(player));
    cr_assert(gm.hasComponent<Projectile>(enemy));
    gm.getComponent<Position2D>(enemy).x = 1200.f;
    shotTerrainSystem(gm);
    gm.getComponent<Position2D>(enemy).x = 910.f;
    shotTerrainSystem(gm);
    cr_assert_not(gm.hasComponent<Projectile>(enemy));
}

Test(weapons, reflect_beam_stops_on_the_first_enemy)
{
    GameManager gm;
    MasterClock clock;
    EnemyDirector director;
    const Entity player = addPlayer(gm);
    std::optional<Entity> front;

    pickUp(gm, player, LaserType::Reflect);
    pickUp(gm, player, LaserType::Reflect);
    fireLaser(gm, *findForce(gm, player));
    cr_assert_eq(countOf(gm, ReflectShot {}), 3 * FORCE_MAX_LEVEL);
    for (auto [shot, reflect, pos, velocity] :
        gm.view<ReflectShot, Position2D, Velocity2D>().getVector())
        if (velocity.y == 0.f &&
            (!front || pos.x > gm.getComponent<Position2D>(*front).x))
            front = shot;
    const auto &pos = gm.getComponent<Position2D>(*front);
    const Entity scout = spawnEnemy(gm, EnemyType::Scout, 0.f, 0.f);
    const auto &box = gm.getComponent<Hitbox>(scout);

    gm.getComponent<Position2D>(scout) = {
        pos.x + REFLECT_WIDTH - box.offsetX - box.width / 2.f,
        pos.y + REFLECT_HEIGHT / 2.f - box.offsetY - box.height / 2.f};
    systemEnemyCombat(gm, director, {});
    reflectSystem(gm, clock);
    cr_assert_eq(countOf(gm, ReflectShot {}), 2 * FORCE_MAX_LEVEL);
    for (auto [_, reflect, velocity] :
        gm.view<ReflectShot, Velocity2D>().getVector())
        cr_assert_neq(velocity.y, 0.f);
}

Test(weapons, terrain_collision_follows_the_sprite_scale)
{
    GameManager gm;
    CollisionMask shape {10, 10, std::vector<bool>(100, true)};
    const Position2D pos {900.f - 10.f * PLAYER_SCALE - 1.f, 500.f};

    addTerrain(gm);
    cr_assert_not(touchesTerrain(gm, pos, shape, PLAYER_SCALE));
    cr_assert(touchesTerrain(gm, pos, shape, SCALE));
}
