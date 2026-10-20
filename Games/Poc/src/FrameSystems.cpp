/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** FrameSystems
*/

#include <stdexcept>
#include <string>
#include <vector>

#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

// Expected API (step 7): a system created without any component type is
// called once per update(), with no argument.
//   RType::SystemId id = gm.createSystem<>([&gm] { ... });

namespace {

    using RType::Entity;
    using RType::GameManager;
    using RType::SystemId;
    using namespace Poc;

    void runsOnceWithoutAnyEntity()
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&calls] { calls++; });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 1);
    }

    void runsOncePerUpdateWhateverTheEntityCount()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 100; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<>([&calls] { calls++; });
        for (int i = 0; i < 10; i++)
            gm.update();
        POC_CHECK(calls == 10);
    }

    void callableWithoutAngleBrackets()
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem([&calls] { calls++; });
        gm.update();
        POC_CHECK(calls == 1);
    }

    void runsInCreationOrderWithOtherSystems()
    {
        GameManager gm;
        std::string order;

        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.createSystem<Health>([&order](Health &) { order += "A"; });
        gm.createSystem<>([&order] { order += "F"; });
        gm.createSystem<Health>([&order](Health &) { order += "B"; });
        gm.update();
        POC_CHECK(order == "AFB");
    }

    void seesChangesOfEarlierSystems()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = -1;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([](Health &h) { h.hp = 42; });
        gm.createSystem<>(
            [&gm, &seen, e] { seen = gm.getComponent<Health>(e).hp; });
        gm.update();
        POC_CHECK(seen == 42);
    }

    void destroyInsideIsDeferred()
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
        POC_CHECK(aliveDuringFrame);
        POC_CHECK(!gm.hasEntity(e));
    }

    void addInsideIsDeferred()
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
        POC_CHECK(!visibleDuringFrame);
        POC_CHECK(gm.getComponent<Health>(e).hp == 5);
    }

    void destroyedFrameSystemStopsRunning()
    {
        GameManager gm;
        int calls = 0;

        SystemId id = gm.createSystem<>([&calls] { calls++; });
        POC_CHECK(gm.hasSystem(id));
        gm.update();
        gm.destroySystem(id);
        POC_CHECK(!gm.hasSystem(id));
        gm.update();
        POC_CHECK(calls == 1);
    }

    void frameSystemDestroysItself()
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
        POC_CHECK(calls == 1);
        POC_CHECK(!gm.hasSystem(self));
    }

    void stateCapturedPersistsAcrossFrames()
    {
        GameManager gm;
        int frame = 0;
        std::vector<int> seen;

        gm.createSystem<>([&frame, &seen] { seen.push_back(++frame); });
        for (int i = 0; i < 3; i++)
            gm.update();
        POC_CHECK((seen == std::vector<int> {1, 2, 3}));
    }

    void nestedUpdateHasNoEffect()
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&gm, &calls] {
            calls++;
            gm.update();
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 1);
    }

    void throwingFrameSystemDoesNotLockQueue()
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
        POC_CHECK_THROWS(gm.update(), std::runtime_error);
        POC_CHECK(gm.hasComponent<Health>(e));
        gm.addComponent<Position>(e, 1.f, 1.f);
        POC_CHECK(gm.hasComponent<Position>(e));
    }

    void collisionsWithViews()
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
        POC_CHECK(hits == 3);
        for (Entity b : bullets)
            POC_CHECK(!gm.hasEntity(b));
        int damaged = 0;
        for (Entity e : enemies)
            damaged += gm.getComponent<Health>(e).hp == 2 ? 1 : 0;
        POC_CHECK(damaged == 3);
        gm.update();
        POC_CHECK(hits == 3);
    }

    void viewOnNeverAddedTypeInsideFrameSystem()
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem<>([&gm, &calls] {
            gm.view<Bullet>().each([&calls](Entity, Bullet &) { calls++; });
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 0);
    }

} // namespace

namespace Poc {

    TestList frameSystemTests()
    {
        return {
            {"[frame system] runs once without any entity",
                runsOnceWithoutAnyEntity},
            {"[frame system] runs once per update whatever the entity count",
                runsOncePerUpdateWhateverTheEntityCount},
            {"[frame system] callable without angle brackets",
                callableWithoutAngleBrackets},
            {"[frame system] runs in creation order with other systems",
                runsInCreationOrderWithOtherSystems},
            {"[frame system] sees the changes of earlier systems",
                seesChangesOfEarlierSystems},
            {"[frame system] a destroy inside is deferred",
                destroyInsideIsDeferred},
            {"[frame system] an add inside is deferred", addInsideIsDeferred},
            {"[frame system] a destroyed frame system stops running",
                destroyedFrameSystemStopsRunning},
            {"[frame system] a frame system destroys itself",
                frameSystemDestroysItself},
            {"[frame system] captured state persists across frames",
                stateCapturedPersistsAcrossFrames},
            {"[frame system] a nested update has no effect",
                nestedUpdateHasNoEffect},
            {"[frame system] a throwing frame system does not lock the queue",
                throwingFrameSystemDoesNotLockQueue},
            {"[frame system] bullets against enemies with views",
                collisionsWithViews},
            {"[frame system] a view on a never added type inside a frame "
             "system",
                viewOnNeverAddedTypeInsideFrameSystem},
        };
    }

} // namespace Poc
