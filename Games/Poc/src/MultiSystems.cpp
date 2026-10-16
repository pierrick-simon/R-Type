/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MultiSystems
*/

#include <cstddef>
#include <random>
#include <vector>

#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

namespace {

    using RType::Entity;
    using RType::GameManager;
    using namespace Poc;

    // Owned identifies the visited entity inside a callback (owner field).

    void visitsOnlyEntitiesWithAllTypes()
    {
        GameManager gm;
        EntitySet expected;
        EntitySet visited;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i % 3 != 0) {
                gm.addComponent<Health>(e, i);
                expected.insert(e);
            }
        }
        gm.createSystem<Owned, Health>(
            [&visited](Owned &o, Health &) { visited.insert(o.owner); });
        gm.update();
        POC_CHECK(visited.size() == expected.size());
        for (Entity e : expected)
            POC_CHECK(visited.contains(e));
    }

    void firstEntityMissingATypeDoesNotStopTheSystem()
    {
        GameManager gm;
        int calls = 0;

        Entity lonely = gm.createEntity();
        gm.addComponent<Owned>(lonely, lonely, 0);
        for (int i = 1; i <= 5; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Owned, Health>(
            [&calls](Owned &, Health &) { calls++; });
        gm.update();
        POC_CHECK(calls == 5);
    }

    void entityMissingATypeInTheMiddle()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 9; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i != 4)
                gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Owned, Health>(
            [&calls](Owned &, Health &) { calls++; });
        gm.update();
        POC_CHECK(calls == 8);
    }

    void componentsBelongToTheSameEntity()
    {
        GameManager gm;
        int calls = 0;
        bool paired = true;

        for (int i = 0; i < 200; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            gm.addComponent<Health>(e, i * 10);
        }
        gm.createSystem<Owned, Health>([&gm, &calls, &paired](
                                           Owned &o, Health &h) {
            calls++;
            if (&gm.getComponent<Health>(o.owner) != &h || h.hp != o.value * 10)
                paired = false;
        });
        gm.update();
        POC_CHECK(calls == 200);
        POC_CHECK(paired);
    }

    void pairingSurvivesRemovalsAndRecycling()
    {
        GameManager gm;
        std::vector<Entity> entities;
        bool paired = true;
        int calls = 0;

        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            gm.addComponent<Owned>(e, e, i);
            entities.push_back(e);
        }
        for (std::size_t i = 0; i < entities.size(); i += 2)
            gm.destroyEntity(entities[i]);
        for (int i = 0; i < 30; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, 1000 + i);
            gm.addComponent<Health>(e, 1000 + i);
        }
        gm.createSystem<Owned, Health>(
            [&gm, &calls, &paired](Owned &o, Health &h) {
                calls++;
                if (&gm.getComponent<Health>(o.owner) != &h || h.hp != o.value)
                    paired = false;
            });
        gm.update();
        POC_CHECK(calls == 80);
        POC_CHECK(paired);
    }

    void modificationsReachTheStoredComponents()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 0.f, 0.f);
        gm.addComponent<Velocity>(e, 2.f, 3.f);
        gm.createSystem<Position, Velocity>([](Position &p, Velocity &v) {
            p.x += v.x;
            p.y += v.y;
            v.x += 1.f;
        });
        gm.update();
        gm.update();
        POC_CHECK(gm.getComponent<Position>(e).x == 5.f);
        POC_CHECK(gm.getComponent<Position>(e).y == 6.f);
        POC_CHECK(gm.getComponent<Velocity>(e).x == 4.f);
    }

    void argumentsFollowTheTemplateOrder()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool ok = false;

        gm.addComponent<Health>(e, 7);
        gm.addComponent<Damage>(e, 3);
        gm.createSystem<Damage, Health>(
            [&ok](Damage &d, Health &h) { ok = d.value == 3 && h.hp == 7; });
        gm.update();
        POC_CHECK(ok);
    }

    void typeOrderDoesNotChangeTheResult()
    {
        GameManager gm;
        EntitySet rareFirst;
        EntitySet commonFirst;

        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i % 25 == 0)
                gm.addComponent<Player>(e);
        }
        gm.createSystem<Player, Owned>(
            [&rareFirst](Player &, Owned &o) { rareFirst.insert(o.owner); });
        gm.createSystem<Owned, Player>([&commonFirst](Owned &o, Player &) {
            commonFirst.insert(o.owner);
        });
        gm.update();
        POC_CHECK(rareFirst.size() == 4);
        POC_CHECK(commonFirst.size() == 4);
        for (Entity e : rareFirst)
            POC_CHECK(commonFirst.contains(e));
    }

    void threeTypes()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 30; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i % 2 == 0)
                gm.addComponent<Health>(e, i);
            if (i % 3 == 0)
                gm.addComponent<Damage>(e, i);
        }
        gm.createSystem<Owned, Health, Damage>(
            [&calls](Owned &o, Health &h, Damage &d) {
                calls++;
                POC_CHECK(o.value % 6 == 0);
                POC_CHECK(h.hp == o.value && d.value == o.value);
            });
        gm.update();
        POC_CHECK(calls == 5);
    }

    void typeNeverAddedDoesNothing()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 0);
        gm.addComponent<Damage>(e, 1);
        gm.update();
        POC_CHECK(calls == 1);
    }

    void emptiedFirstArrayDoesNothing()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.addComponent<Damage>(e, 1);
        gm.removeComponent<Health>(e);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 0);
    }

    void multiSystemDoesNotCopyComponents()
    {
        Tracked::alive = 0;
        GameManager gm;
        int seenAlive = -1;

        for (int i = 0; i < 3; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Tracked>(e, i);
            gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Health, Tracked>(
            [&seenAlive](Health &, Tracked &) { seenAlive = Tracked::alive; });
        gm.update();
        POC_CHECK(seenAlive == 3);
        POC_CHECK(Tracked::alive == 3);
    }

    void constParameters()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        float seen = 0.f;

        gm.addComponent<Position>(e, 4.f, 0.f);
        gm.addComponent<Velocity>(e, 1.f, 0.f);
        gm.createSystem<Position, Velocity>(
            [&seen](
                const Position &p, const Velocity &v) { seen = p.x + v.x; });
        gm.update();
        POC_CHECK(seen == 5.f);
    }

    void followsAddsAndRemovesAcrossFrames()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        gm.addComponent<Damage>(gm.createEntity(), 0);
        gm.update();
        POC_CHECK(calls == 0);
        gm.addComponent<Damage>(e, 1);
        gm.update();
        POC_CHECK(calls == 1);
        gm.removeComponent<Health>(e);
        gm.update();
        POC_CHECK(calls == 1);
        gm.addComponent<Health>(e, 2);
        gm.update();
        POC_CHECK(calls == 2);
    }

    void removalDuringUpdateIsDeferred()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Owned, Health>([&gm, &calls](Owned &o, Health &) {
            calls++;
            gm.removeComponent<Health>(o.owner);
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(calls == 10);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 0);
        gm.update();
        POC_CHECK(calls == 10);
    }

    void destroyDuringUpdateStaysVisibleThisFrame()
    {
        GameManager gm;
        int firstCalls = 0;
        int secondCalls = 0;

        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            gm.addComponent<Health>(e, i);
        }
        gm.createSystem<Owned, Health>([&gm, &firstCalls](Owned &o, Health &) {
            firstCalls++;
            gm.destroyEntity(o.owner);
        });
        gm.createSystem<Health, Owned>(
            [&secondCalls](Health &, Owned &) { secondCalls++; });
        gm.update();
        POC_CHECK(firstCalls == 10);
        POC_CHECK(secondCalls == 10);
        gm.update();
        POC_CHECK(firstCalls == 10);
        POC_CHECK(secondCalls == 10);
    }

    void recycledSlotDoesNotInheritComponents()
    {
        GameManager gm;
        int calls = 0;

        Entity old = gm.createEntity();
        gm.addComponent<Owned>(old, old, 0);
        gm.addComponent<Health>(old, 0);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        POC_CHECK(indexOf(reused) == indexOf(old));
        gm.addComponent<Owned>(reused, reused, 1);
        gm.createSystem<Owned, Health>(
            [&calls](Owned &, Health &) { calls++; });
        gm.update();
        POC_CHECK(calls == 0);
    }

    void denseKnowsTheOwnerOfEachComponent()
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 50; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
        }
        for (std::size_t i = 0; i < entities.size(); i += 3)
            gm.destroyEntity(entities[i]);
        for (int i = 0; i < 10; i++)
            gm.addComponent<Health>(gm.createEntity(), 100 + i);
        std::size_t count = 0;
        for (auto &pack : gm.getArray<Health>().getDense()) {
            count++;
            POC_CHECK(gm.hasEntity(pack.entity()));
            POC_CHECK(
                &gm.getComponent<Health>(pack.entity()) == &pack.component);
        }
        POC_CHECK(count == gm.getArray<Health>().nbComponent());
    }

    void randomSubsetsMatchModel()
    {
        GameManager gm;
        std::mt19937 rng(1234);
        std::vector<Entity> entities;
        std::vector<bool> hasHealth;
        std::vector<bool> hasDamage;

        for (int i = 0; i < 1000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            entities.push_back(e);
            hasHealth.push_back(rng() % 2 == 0);
            hasDamage.push_back(rng() % 3 == 0);
            if (hasHealth.back())
                gm.addComponent<Health>(e, i);
            if (hasDamage.back())
                gm.addComponent<Damage>(e, i);
        }
        EntitySet visited;
        gm.createSystem<Damage, Owned, Health>(
            [&visited](
                Damage &, Owned &o, Health &) { visited.insert(o.owner); });
        gm.update();
        std::size_t expected = 0;
        for (std::size_t i = 0; i < entities.size(); i++) {
            bool match = hasHealth[i] && hasDamage[i];
            expected += match ? 1 : 0;
            POC_CHECK(visited.contains(entities[i]) == match);
        }
        POC_CHECK(visited.size() == expected);
    }

} // namespace

namespace Poc {

    TestList multiSystemTests()
    {
        return {
            {"[multi] visits only entities that have all the types",
                visitsOnlyEntitiesWithAllTypes},
            {"[multi] a first entity missing a type does not stop the system",
                firstEntityMissingATypeDoesNotStopTheSystem},
            {"[multi] an entity missing a type in the middle is skipped",
                entityMissingATypeInTheMiddle},
            {"[multi] the components belong to the same entity",
                componentsBelongToTheSameEntity},
            {"[multi] pairing survives removals and recycling",
                pairingSurvivesRemovalsAndRecycling},
            {"[multi] modifications reach the stored components",
                modificationsReachTheStoredComponents},
            {"[multi] arguments follow the template order",
                argumentsFollowTheTemplateOrder},
            {"[multi] the type order does not change the result",
                typeOrderDoesNotChangeTheResult},
            {"[multi] a system on three types", threeTypes},
            {"[multi] a type never added does nothing",
                typeNeverAddedDoesNothing},
            {"[multi] an emptied first array does nothing",
                emptiedFirstArrayDoesNothing},
            {"[multi] a system does not copy the components",
                multiSystemDoesNotCopyComponents},
            {"[multi] const parameters", constParameters},
            {"[multi] follows adds and removes across frames",
                followsAddsAndRemovesAcrossFrames},
            {"[multi] a removal during update is deferred",
                removalDuringUpdateIsDeferred},
            {"[multi] a destroyed entity stays visible until the frame ends",
                destroyDuringUpdateStaysVisibleThisFrame},
            {"[multi] a recycled slot does not inherit components",
                recycledSlotDoesNotInheritComponents},
            {"[multi] the dense knows the owner of each component",
                denseKnowsTheOwnerOfEachComponent},
            {"[multi] 1000 random subsets match a model",
                randomSubsetsMatchModel},
        };
    }

} // namespace Poc
