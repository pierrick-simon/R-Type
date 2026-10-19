/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GmSystems
*/

#include <string>
#include <vector>

#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

// Expected API: the callback of a system may take the GameManager as first
// parameter, for per-entity systems and for frame systems.
//   gm.createSystem<A, B>([](RType::GameManager &gm, A &a, B &b) { ... });
//   gm.createSystem([](RType::GameManager &gm) { ... });

namespace {

    using RType::Entity;
    using RType::GameManager;
    using RType::SystemId;
    using namespace Poc;

    int freeEntityCalls = 0;
    int freeFrameCalls = 0;
    const GameManager *freeSeenGm = nullptr;

    void freeLifetimeSystem(GameManager &gm, Lifetime &l)
    {
        freeEntityCalls++;
        freeSeenGm = &gm;
        if (--l.ticks <= 0)
            gm.destroyEntity(l.owner);
    }

    void freeFrameSystem(GameManager &gm)
    {
        freeFrameCalls++;
        freeSeenGm = &gm;
    }

    void registerFromHelper(GameManager &gm, int &calls)
    {
        auto local = [&calls](GameManager &, Health &) {
            calls++;
        };
        gm.createSystem<Health>(local);
    }

    void receivesTheSameGameManager()
    {
        GameManager gm;
        bool same = true;
        int calls = 0;

        for (int i = 0; i < 5; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<Health>(
            [&gm, &same, &calls](GameManager &received, Health &) {
                calls++;
                same = same && &received == &gm;
            });
        gm.update();
        POC_CHECK(calls == 5);
        POC_CHECK(same);
    }

    void severalTypesWithGameManager()
    {
        GameManager gm;
        bool paired = true;
        int calls = 0;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i % 2 == 0)
                gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Owned, Health>(
            [&paired, &calls](GameManager &g, Owned &o, Health &h) {
                calls++;
                if (&g.getComponent<Health>(o.owner) != &h || h.hp != o.value)
                    paired = false;
            });
        gm.update();
        POC_CHECK(calls == 10);
        POC_CHECK(paired);
    }

    void freeFunctionPerEntity()
    {
        GameManager gm;
        std::vector<Entity> entities;

        freeEntityCalls = 0;
        freeSeenGm = nullptr;
        for (int i = 0; i < 4; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Lifetime>(e, e, i % 2 + 1);
            entities.push_back(e);
        }
        gm.createSystem<Lifetime>(freeLifetimeSystem);
        gm.update();
        POC_CHECK(freeEntityCalls == 4);
        POC_CHECK(freeSeenGm == &gm);
        for (std::size_t i = 0; i < entities.size(); i++)
            POC_CHECK(gm.hasEntity(entities[i]) == (i % 2 == 1));
    }

    void frameSystemLambdaWithGameManager()
    {
        GameManager gm;
        int calls = 0;
        bool same = false;

        for (int i = 0; i < 10; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem([&gm, &calls, &same](GameManager &received) {
            calls++;
            same = &received == &gm;
        });
        gm.update();
        gm.update();
        POC_CHECK(calls == 2);
        POC_CHECK(same);
    }

    void frameSystemFreeFunction()
    {
        GameManager gm;

        freeFrameCalls = 0;
        freeSeenGm = nullptr;
        gm.createSystem(freeFrameSystem);
        gm.update();
        POC_CHECK(freeFrameCalls == 1);
        POC_CHECK(freeSeenGm == &gm);
    }

    void destroyThroughReceivedGmIsDeferred()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool aliveDuringFrame = false;

        gm.addComponent<Lifetime>(e, e, 0);
        gm.createSystem<Lifetime>(
            [&aliveDuringFrame](GameManager &g, Lifetime &l) {
                g.destroyEntity(l.owner);
                aliveDuringFrame = g.hasEntity(l.owner);
            });
        gm.update();
        POC_CHECK(aliveDuringFrame);
        POC_CHECK(!gm.hasEntity(e));
    }

    void collisionsThroughReceivedGm()
    {
        GameManager gm;
        std::vector<Entity> bullets;
        std::vector<Entity> enemies;

        for (int i = 0; i < 4; i++) {
            Entity enemy = gm.createEntity();
            gm.addComponent<Enemy>(enemy);
            gm.addComponent<Position>(enemy, static_cast<float>(i) * 10.f, 0.f);
            gm.addComponent<Health>(enemy, 3);
            enemies.push_back(enemy);
        }
        for (int i = 0; i < 3; i++) {
            Entity bullet = gm.createEntity();
            gm.addComponent<Bullet>(bullet);
            gm.addComponent<Position>(
                bullet, static_cast<float>(i) * 20.f, 0.f);
            gm.addComponent<Damage>(bullet, 1);
            bullets.push_back(bullet);
        }
        gm.createSystem([](GameManager &g) {
            g.view<Bullet, Position, Damage>().each(
                [&g](Entity bullet, Bullet &, Position &bp, Damage &d) {
                    g.view<Enemy, Position, Health>().each(
                        [&](Entity, Enemy &, Position &ep, Health &h) {
                            if (ep.x != bp.x)
                                return;
                            h.hp -= d.value;
                            g.destroyEntity(bullet);
                        });
                });
        });
        gm.update();
        POC_CHECK(!gm.hasEntity(bullets[0]));
        POC_CHECK(!gm.hasEntity(bullets[1]));
        POC_CHECK(gm.hasEntity(bullets[2]));
        POC_CHECK(gm.getComponent<Health>(enemies[0]).hp == 2);
        POC_CHECK(gm.getComponent<Health>(enemies[1]).hp == 3);
        POC_CHECK(gm.getComponent<Health>(enemies[2]).hp == 2);
        POC_CHECK(gm.getComponent<Health>(enemies[3]).hp == 3);
    }

    void allFormsCoexistInCreationOrder()
    {
        GameManager gm;
        std::string order;

        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.createSystem<Health>([&order](Health &) { order += "A"; });
        gm.createSystem<Health>(
            [&order](GameManager &, Health &) { order += "B"; });
        gm.createSystem([&order] { order += "C"; });
        gm.createSystem([&order](GameManager &) { order += "D"; });
        gm.update();
        POC_CHECK(order == "ABCD");
    }

    void functionIsKeptByValue()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        registerFromHelper(gm, calls);
        gm.update();
        gm.update();
        POC_CHECK(calls == 6);
    }

    void eachSystemReceivesItsOwnGameManager()
    {
        GameManager first;
        GameManager second;
        const GameManager *seenByFirst = nullptr;
        const GameManager *seenBySecond = nullptr;

        first.createSystem(
            [&seenByFirst](GameManager &g) { seenByFirst = &g; });
        second.createSystem(
            [&seenBySecond](GameManager &g) { seenBySecond = &g; });
        first.update();
        second.update();
        POC_CHECK(seenByFirst == &first);
        POC_CHECK(seenBySecond == &second);
    }

    void destroyedGmSystemStopsRunning()
    {
        GameManager gm;
        int calls = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        SystemId id = gm.createSystem<Health>(
            [&calls](GameManager &, Health &) { calls++; });
        POC_CHECK(gm.hasSystem(id));
        gm.update();
        gm.destroySystem(id);
        POC_CHECK(!gm.hasSystem(id));
        gm.update();
        POC_CHECK(calls == 1);
    }

    void gmSystemDestroysItself()
    {
        GameManager gm;
        int calls = 0;
        SystemId self = 0;

        self = gm.createSystem([&calls, &self](GameManager &g) {
            calls++;
            g.destroySystem(self);
        });
        gm.update();
        gm.update();
        POC_CHECK(calls == 1);
        POC_CHECK(!gm.hasSystem(self));
    }

    void nestedUpdateThroughReceivedGmHasNoEffect()
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem([&calls](GameManager &g) {
            calls++;
            g.update();
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 1);
    }

    void constComponentWithGm()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = 0;

        gm.addComponent<Health>(e, 9);
        gm.createSystem<Health>(
            [&seen](GameManager &, const Health &h) { seen = h.hp; });
        gm.update();
        POC_CHECK(seen == 9);
    }

    void gmFormDoesNotCopyComponents()
    {
        Tracked::alive = 0;
        GameManager gm;
        int seenAlive = -1;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Tracked>(gm.createEntity(), i);
        gm.createSystem<Tracked>([&seenAlive](GameManager &, Tracked &) {
            seenAlive = Tracked::alive;
        });
        gm.update();
        POC_CHECK(seenAlive == 3);
        POC_CHECK(Tracked::alive == 3);
    }

} // namespace

namespace Poc {

    TestList gmSystemTests()
    {
        return {
            {"[gm arg] receives the same GameManager",
                receivesTheSameGameManager},
            {"[gm arg] several types with the GameManager",
                severalTypesWithGameManager},
            {"[gm arg] a free function per entity", freeFunctionPerEntity},
            {"[gm arg] a frame system lambda",
                frameSystemLambdaWithGameManager},
            {"[gm arg] a frame system free function", frameSystemFreeFunction},
            {"[gm arg] a destroy through the received gm is deferred",
                destroyThroughReceivedGmIsDeferred},
            {"[gm arg] collisions through the received gm",
                collisionsThroughReceivedGm},
            {"[gm arg] all forms coexist in creation order",
                allFormsCoexistInCreationOrder},
            {"[gm arg] the function is kept by value", functionIsKeptByValue},
            {"[gm arg] each system receives its own GameManager",
                eachSystemReceivesItsOwnGameManager},
            {"[gm arg] a destroyed gm system stops running",
                destroyedGmSystemStopsRunning},
            {"[gm arg] a gm system destroys itself", gmSystemDestroysItself},
            {"[gm arg] a nested update through the gm has no effect",
                nestedUpdateThroughReceivedGmHasNoEffect},
            {"[gm arg] const component", constComponentWithGm},
            {"[gm arg] does not copy the components",
                gmFormDoesNotCopyComponents},
        };
    }

} // namespace Poc
