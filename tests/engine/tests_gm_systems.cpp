/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_gm_systems
*/

#include <string>
#include <vector>

#include <criterion/criterion.h>

#include "Entity.hpp"
#include "GameManager.hpp"

// Expected API: the callback of a system may take the GameManager as first
// parameter, for per-entity systems and for frame systems.
//   gm.createSystem<A, B>([](RType::GameManager &gm, A &a, B &b) { ... });
//   gm.createSystem([](RType::GameManager &gm) { ... });

namespace {

    struct Enemy {};

    struct Bullet {};

    struct Owned {
        RType::Entity owner;
        int value;
    };

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

    struct Lifetime {
        RType::Entity owner;
        int ticks;
    };

    struct Tracked {
        static inline int alive = 0;

        int value;

        explicit Tracked(int v = 0) :
            value(v)
        {
            alive++;
        }

        Tracked(const Tracked &other) :
            value(other.value)
        {
            alive++;
        }

        Tracked(Tracked &&other) noexcept :
            value(other.value)
        {
            alive++;
        }

        Tracked &operator=(const Tracked &) = default;
        Tracked &operator=(Tracked &&) noexcept = default;

        ~Tracked()
        {
            alive--;
        }
    };

    using RType::Entity;
    using RType::GameManager;
    using RType::SystemId;

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

    Test(gm_systems, receivesTheSameGameManager,
        .description = "[gm arg] receives the same GameManager")
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
        cr_assert(calls == 5);
        cr_assert(same);
    }

    Test(gm_systems, severalTypesWithGameManager,
        .description = "[gm arg] several types with the GameManager")
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
        cr_assert(calls == 10);
        cr_assert(paired);
    }

    Test(gm_systems, freeFunctionPerEntity,
        .description = "[gm arg] a free function per entity")
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
        cr_assert(freeEntityCalls == 4);
        cr_assert(freeSeenGm == &gm);
        for (std::size_t i = 0; i < entities.size(); i++)
            cr_assert(gm.hasEntity(entities[i]) == (i % 2 == 1));
    }

    Test(gm_systems, frameSystemLambdaWithGameManager,
        .description = "[gm arg] a frame system lambda")
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
        cr_assert(calls == 2);
        cr_assert(same);
    }

    Test(gm_systems, frameSystemFreeFunction,
        .description = "[gm arg] a frame system free function")
    {
        GameManager gm;

        freeFrameCalls = 0;
        freeSeenGm = nullptr;
        gm.createSystem(freeFrameSystem);
        gm.update();
        cr_assert(freeFrameCalls == 1);
        cr_assert(freeSeenGm == &gm);
    }

    Test(gm_systems, destroyThroughReceivedGmIsDeferred,
        .description = "[gm arg] a destroy through the received gm is deferred")
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
        cr_assert(aliveDuringFrame);
        cr_assert(!gm.hasEntity(e));
    }

    Test(gm_systems, collisionsThroughReceivedGm,
        .description = "[gm arg] collisions through the received gm")
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
        cr_assert(!gm.hasEntity(bullets[0]));
        cr_assert(!gm.hasEntity(bullets[1]));
        cr_assert(gm.hasEntity(bullets[2]));
        cr_assert(gm.getComponent<Health>(enemies[0]).hp == 2);
        cr_assert(gm.getComponent<Health>(enemies[1]).hp == 3);
        cr_assert(gm.getComponent<Health>(enemies[2]).hp == 2);
        cr_assert(gm.getComponent<Health>(enemies[3]).hp == 3);
    }

    Test(gm_systems, allFormsCoexistInCreationOrder,
        .description = "[gm arg] all forms coexist in creation order")
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
        cr_assert(order == "ABCD");
    }

    Test(gm_systems, functionIsKeptByValue,
        .description = "[gm arg] the function is kept by value")
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        registerFromHelper(gm, calls);
        gm.update();
        gm.update();
        cr_assert(calls == 6);
    }

    Test(gm_systems, eachSystemReceivesItsOwnGameManager,
        .description = "[gm arg] each system receives its own GameManager")
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
        cr_assert(seenByFirst == &first);
        cr_assert(seenBySecond == &second);
    }

    Test(gm_systems, destroyedGmSystemStopsRunning,
        .description = "[gm arg] a destroyed gm system stops running")
    {
        GameManager gm;
        int calls = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        SystemId id = gm.createSystem<Health>(
            [&calls](GameManager &, Health &) { calls++; });
        cr_assert(gm.hasSystem(id));
        gm.update();
        gm.destroySystem(id);
        cr_assert(!gm.hasSystem(id));
        gm.update();
        cr_assert(calls == 1);
    }

    Test(gm_systems, gmSystemDestroysItself,
        .description = "[gm arg] a gm system destroys itself")
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
        cr_assert(calls == 1);
        cr_assert(!gm.hasSystem(self));
    }

    Test(gm_systems, nestedUpdateThroughReceivedGmHasNoEffect,
        .description = "[gm arg] a nested update through the gm has no effect")
    {
        GameManager gm;
        int calls = 0;

        gm.createSystem([&calls](GameManager &g) {
            calls++;
            g.update();
        });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 1);
    }

    Test(gm_systems, constComponentWithGm,
        .description = "[gm arg] const component")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = 0;

        gm.addComponent<Health>(e, 9);
        gm.createSystem<Health>(
            [&seen](GameManager &, const Health &h) { seen = h.hp; });
        gm.update();
        cr_assert(seen == 9);
    }

    Test(gm_systems, gmFormDoesNotCopyComponents,
        .description = "[gm arg] does not copy the components")
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
        cr_assert(seenAlive == 3);
        cr_assert(Tracked::alive == 3);
    }

} // namespace
