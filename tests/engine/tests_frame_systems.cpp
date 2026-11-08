/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_frame_systems
*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <criterion/criterion.h>

#include "Entity.hpp"
#include "GameManager.hpp"

// Expected API (step 7): a system created without any component type is
// called once per update(), with no argument.
//   RType::SystemId id = gm.createSystem<>([&gm] { ... });

namespace {

    struct Enemy {};

    struct Bullet {};

    struct Position {
        float x;
        float y;
    };

    struct Health {
        int hp;
    };

    struct Damage {
        int value;
    };

    class EntitySet {
    public:
        [[nodiscard]] bool contains(const RType::Entity &entity) const
        {
            return std::find(_entities.begin(), _entities.end(), entity) !=
                _entities.end();
        }

        void insert(const RType::Entity &entity)
        {
            if (!contains(entity))
                _entities.push_back(entity);
        }

        void clear()
        {
            _entities.clear();
        }

        [[nodiscard]] std::size_t size() const
        {
            return _entities.size();
        }

        [[nodiscard]] auto begin() const
        {
            return _entities.begin();
        }

        [[nodiscard]] auto end() const
        {
            return _entities.end();
        }

    private:
        std::vector<RType::Entity> _entities;
    };

    using RType::Entity;
    using RType::GameManager;
    using RType::SystemId;

    Test(frame_systems, runsOnceWithoutAnyEntity,
        .description = "[frame system] runs once without any entity")
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&calls] { calls++; });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 1);
    }

    Test(frame_systems, runsOncePerUpdateWhateverTheEntityCount,
        .description =
            "[frame system] runs once per update whatever the entity count")
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 100; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<>([&calls] { calls++; });
        for (int i = 0; i < 10; i++)
            gm.update();
        cr_assert(calls == 10);
    }

    Test(frame_systems, callableWithoutAngleBrackets,
        .description = "[frame system] callable without angle brackets")
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem([&calls] { calls++; });
        gm.update();
        cr_assert(calls == 1);
    }

    Test(frame_systems, runsInCreationOrderWithOtherSystems,
        .description =
            "[frame system] runs in creation order with other systems")
    {
        GameManager gm;
        std::string order;

        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.createSystem<Health>([&order](Health &) { order += "A"; });
        gm.createSystem<>([&order] { order += "F"; });
        gm.createSystem<Health>([&order](Health &) { order += "B"; });
        gm.update();
        cr_assert(order == "AFB");
    }

    Test(frame_systems, seesChangesOfEarlierSystems,
        .description = "[frame system] sees the changes of earlier systems")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = -1;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([](Health &h) { h.hp = 42; });
        gm.createSystem<>(
            [&gm, &seen, e] { seen = gm.getComponent<Health>(e).hp; });
        gm.update();
        cr_assert(seen == 42);
    }

    Test(frame_systems, destroyInsideIsDeferred,
        .description = "[frame system] a destroy inside is deferred")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool aliveDuringFrame = false;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<>([&gm, &aliveDuringFrame, e] {
            if (!gm.hasEntity(e))
                return;
            gm.destroyEntity(e);
            aliveDuringFrame = gm.hasEntity(e);
        });
        gm.update();
        cr_assert(aliveDuringFrame);
        cr_assert(!gm.hasEntity(e));
    }

    Test(frame_systems, addInsideIsDeferred,
        .description = "[frame system] an add inside is deferred")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool visibleDuringFrame = true;
        bool added = false;

        gm.createSystem<>([&gm, &visibleDuringFrame, &added, e] {
            if (added)
                return;
            added = true;
            gm.addComponent<Health>(e, 5);
            visibleDuringFrame = gm.hasComponent<Health>(e);
        });
        gm.update();
        cr_assert(!visibleDuringFrame);
        cr_assert(gm.getComponent<Health>(e).hp == 5);
    }

    Test(frame_systems, destroyedFrameSystemStopsRunning,
        .description = "[frame system] a destroyed frame system stops running")
    {
        GameManager gm;
        int calls = 0;

        SystemId id = gm.createSystem<>([&calls] { calls++; });
        cr_assert(gm.hasSystem(id));
        gm.update();
        gm.destroySystem(id);
        cr_assert(!gm.hasSystem(id));
        gm.update();
        cr_assert(calls == 1);
    }

    Test(frame_systems, frameSystemDestroysItself,
        .description = "[frame system] a frame system destroys itself")
    {
        GameManager gm;
        int calls = 0;
        SystemId self = 0;

        self = gm.createSystem<>([&gm, &calls, &self] {
            calls++;
            gm.destroySystem(self);
        });
        gm.update();
        gm.update();
        cr_assert(calls == 1);
        cr_assert(!gm.hasSystem(self));
    }

    Test(frame_systems, stateCapturedPersistsAcrossFrames,
        .description = "[frame system] captured state persists across frames")
    {
        GameManager gm;
        int frame = 0;
        std::vector<int> seen;

        gm.createSystem<>([&frame, &seen] { seen.push_back(++frame); });
        for (int i = 0; i < 3; i++)
            gm.update();
        cr_assert((seen == std::vector<int> {1, 2, 3}));
    }

    Test(frame_systems, nestedUpdateHasNoEffect,
        .description = "[frame system] a nested update has no effect")
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&gm, &calls] {
            calls++;
            gm.update();
        });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 1);
    }

    Test(frame_systems, throwingFrameSystemDoesNotLockQueue,
        .description =
            "[frame system] a throwing frame system does not lock the queue")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool thrown = false;

        gm.createSystem<>([&gm, &thrown, e] {
            if (thrown)
                return;
            thrown = true;
            gm.addComponent<Health>(e, 3);
            throw std::runtime_error("frame system failure");
        });
        cr_assert_throw(gm.update(), std::runtime_error);
        cr_assert(gm.hasComponent<Health>(e));
        gm.addComponent<Position>(e, 1.f, 1.f);
        cr_assert(gm.hasComponent<Position>(e));
    }

    Test(frame_systems, collisionsWithViews,
        .description = "[frame system] bullets against enemies with views")
    {
        GameManager gm;
        EntitySet bullets;
        EntitySet enemies;

        for (int i = 0; i < 5; i++) {
            Entity enemy = gm.createEntity();
            gm.addComponent<Enemy>(enemy);
            gm.addComponent<Position>(enemy, static_cast<float>(i) * 10.f, 0.f);
            gm.addComponent<Health>(enemy, 3);
            enemies.insert(enemy);
        }
        for (int i = 0; i < 3; i++) {
            Entity bullet = gm.createEntity();
            gm.addComponent<Bullet>(bullet);
            gm.addComponent<Position>(
                bullet, static_cast<float>(i) * 20.f, 0.f);
            gm.addComponent<Damage>(bullet, 1);
            bullets.insert(bullet);
        }
        int hits = 0;
        gm.createSystem<>([&gm, &hits] {
            gm.view<Bullet, Position, Damage>().each(
                [&gm, &hits](Entity bullet, Bullet &, Position &bp, Damage &d) {
                    gm.view<Enemy, Position, Health>().each(
                        [&](Entity, Enemy &, Position &ep, Health &h) {
                            if (ep.x != bp.x)
                                return;
                            h.hp -= d.value;
                            hits++;
                            gm.destroyEntity(bullet);
                        });
                });
        });
        gm.update();
        cr_assert(hits == 3);
        for (Entity b : bullets)
            cr_assert(!gm.hasEntity(b));
        int damaged = 0;
        for (Entity e : enemies)
            damaged += gm.getComponent<Health>(e).hp == 2 ? 1 : 0;
        cr_assert(damaged == 3);
        gm.update();
        cr_assert(hits == 3);
    }

    Test(frame_systems, viewOnNeverAddedTypeInsideFrameSystem,
        .description =
            "[frame system] a view on a never added type inside a frame system")
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&gm, &calls] {
            gm.view<Bullet>().each([&calls](Entity, Bullet &) { calls++; });
        });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 0);
    }

} // namespace
