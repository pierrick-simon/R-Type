/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Systems
*/

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

namespace {

    using RType::Entity;
    using RType::GameManager;
    using RType::SystemId;
    using namespace Poc;

    void updateWithoutSystems()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 5);
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(gm.getComponent<Health>(e).hp == 5);
    }

    void systemModifiesStoredComponents()
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
        }
        gm.createSystem<Health>([](Health &h) { h.hp += 1000; });
        gm.update();
        for (std::size_t i = 0; i < entities.size(); i++)
            POC_CHECK(gm.getComponent<Health>(entities[i]).hp ==
                static_cast<int>(i) + 1000);
    }

    void systemDoesNotCopyComponents()
    {
        Tracked::alive = 0;
        GameManager gm;
        int seenAlive = -1;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Tracked>(gm.createEntity(), i);
        gm.createSystem<Tracked>(
            [&seenAlive](Tracked &) { seenAlive = Tracked::alive; });
        gm.update();
        POC_CHECK(seenAlive == 3);
        POC_CHECK(Tracked::alive == 3);
    }

    void systemOnlyTouchesItsType()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        gm.addComponent<Velocity>(e, 1.f, 1.f);
        gm.createSystem<Position>([](Position &p) { p.x = 42.f; });
        gm.update();
        POC_CHECK(gm.getComponent<Position>(e).x == 42.f);
        POC_CHECK(gm.getComponent<Velocity>(e).x == 1.f);
    }

    void systemVisitsEachComponentOnce()
    {
        GameManager gm;
        std::map<int, int> visits;

        for (int i = 0; i < 500; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<Health>([&visits](Health &h) { visits[h.hp]++; });
        gm.update();
        POC_CHECK(visits.size() == 500);
        for (const auto &[_, count] : visits)
            POC_CHECK(count == 1);
    }

    void systemsRunInCreationOrder()
    {
        GameManager gm;
        std::vector<int> order;

        gm.addComponent<Health>(gm.createEntity(), 0);
        gm.createSystem<Health>([&order](Health &) { order.push_back(1); });
        gm.createSystem<Health>([&order](Health &) { order.push_back(2); });
        gm.createSystem<Health>([&order](Health &) { order.push_back(3); });
        gm.update();
        gm.update();
        POC_CHECK((order == std::vector<int> {1, 2, 3, 1, 2, 3}));
    }

    void laterSystemSeesEarlierChanges()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = -1;

        gm.addComponent<Health>(e, 0);
        gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        gm.createSystem<Health>([&seen](Health &h) { seen = h.hp; });
        gm.update();
        POC_CHECK(seen == 1);
        gm.update();
        POC_CHECK(seen == 2);
    }

    void destroyedSystemStopsRunning()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 0);
        auto sys = gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        gm.update();
        gm.destroySystem(sys);
        gm.update();
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
    }

    void orderKeptAfterDestroyAndCreate()
    {
        GameManager gm;
        std::vector<char> order;

        gm.addComponent<Health>(gm.createEntity(), 0);
        gm.createSystem<Health>([&order](Health &) { order.push_back('A'); });
        auto b = gm.createSystem<Health>(
            [&order](Health &) { order.push_back('B'); });
        gm.createSystem<Health>([&order](Health &) { order.push_back('C'); });
        gm.destroySystem(b);
        gm.createSystem<Health>([&order](Health &) { order.push_back('D'); });
        gm.update();
        POC_CHECK((order == std::vector<char> {'A', 'C', 'D'}));
    }

    void handlesStayValidAfterOtherDestroys()
    {
        GameManager gm;
        std::vector<char> order;
        std::vector<SystemId> handles;

        gm.addComponent<Health>(gm.createEntity(), 0);
        for (char c = 'A'; c <= 'E'; c++)
            handles.push_back(gm.createSystem<Health>(
                [&order, c](Health &) { order.push_back(c); }));
        gm.destroySystem(handles[1]);
        gm.destroySystem(handles[3]);
        gm.destroySystem(handles[0]);
        gm.update();
        POC_CHECK((order == std::vector<char> {'C', 'E'}));
    }

    void systemCreatedBeforeAnyComponent()
    {
        GameManager gm;
        int visits = 0;

        gm.createSystem<Health>([&visits](Health &) { visits++; });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(visits == 0);
        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.update();
        POC_CHECK(visits == 1);
    }

    void systemFollowsAddsAndRemoves()
    {
        GameManager gm;
        std::vector<Entity> entities;
        int visits = 0;

        gm.createSystem<Health>([&visits](Health &) { visits++; });
        for (int i = 0; i < 10; i++) {
            entities.push_back(gm.createEntity());
            gm.addComponent<Health>(entities.back(), i);
        }
        gm.update();
        POC_CHECK(visits == 10);
        visits = 0;
        gm.removeComponent<Health>(entities[0]);
        gm.destroyEntity(entities[5]);
        entities.erase(entities.begin() + 5);
        gm.addComponent<Health>(gm.createEntity(), 99);
        gm.update();
        POC_CHECK(visits == 9);
        visits = 0;
        for (Entity e : entities)
            if (gm.hasEntity(e))
                gm.destroyEntity(e);
        gm.update();
        POC_CHECK(visits == 1);
    }

    void systemOnEmptiedArray()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int visits = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([&visits](Health &) { visits++; });
        gm.removeComponent<Health>(e);
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(visits == 0);
    }

    void repeatedUpdatesAccumulate()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 0);
        gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        for (int i = 0; i < 1000; i++)
            gm.update();
        POC_CHECK(gm.getComponent<Health>(e).hp == 1000);
    }

    void twoSystemsOnDifferentTypes()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 0.f, 0.f);
        gm.addComponent<Health>(e, 10);
        gm.createSystem<Position>([](Position &p) { p.x += 1.f; });
        gm.createSystem<Health>([](Health &h) { h.hp -= 1; });
        gm.update();
        gm.update();
        POC_CHECK(gm.getComponent<Position>(e).x == 2.f);
        POC_CHECK(gm.getComponent<Health>(e).hp == 8);
    }

    void systemAddsOtherTypeDuringUpdate()
    {
        GameManager gm;
        std::vector<Entity> spawned;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
        }
        gm.createSystem<Owned>([&gm, &spawned](Owned &o) {
            Entity bullet = gm.createEntity();
            gm.addComponent<Health>(bullet, o.value);
            spawned.push_back(bullet);
        });
        gm.update();
        POC_CHECK(spawned.size() == 20);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 20);
        for (auto &s : spawned)
            POC_CHECK(gm.hasComponent<Health>(s));
    }

    void systemAddsWithTemporaryArguments()
    {
        GameManager gm;
        std::vector<std::pair<Entity, int>> spawned;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
        }
        gm.createSystem<Owned>([&gm, &spawned](Owned &o) {
            Entity e = gm.createEntity();
            gm.addComponent<Name>(
                e, std::string(64, 'x') + std::to_string(o.value));
            gm.addComponent<Health>(e, o.value * 10);
            spawned.emplace_back(e, o.value);
        });
        gm.update();
        POC_CHECK(spawned.size() == 20);
        for (const auto &[e, value] : spawned) {
            POC_CHECK(gm.getComponent<Name>(e).value ==
                std::string(64, 'x') + std::to_string(value));
            POC_CHECK(gm.getComponent<Health>(e).hp == value * 10);
        }
    }

    void systemModifiesOtherTypeThroughOwner()
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            gm.addComponent<Health>(e, 0);
            entities.push_back(e);
        }
        gm.createSystem<Owned>([&gm](Owned &o) {
            gm.getComponent<Health>(o.owner).hp = o.value * 2;
        });
        gm.update();
        for (std::size_t i = 0; i < entities.size(); i++)
            POC_CHECK(gm.getComponent<Health>(entities[i]).hp ==
                static_cast<int>(i) * 2);
    }

    void systemDestroysOtherSystem()
    {
        GameManager gm;
        std::vector<char> order;
        SystemId c = 0;
        bool destroyed = false;

        gm.addComponent<Health>(gm.createEntity(), 0);
        gm.createSystem<Health>([&](Health &) {
            order.push_back('A');
            if (!destroyed)
                gm.destroySystem(c);
            destroyed = true;
        });
        gm.createSystem<Health>([&order](Health &) { order.push_back('B'); });
        c = gm.createSystem<Health>(
            [&order](Health &) { order.push_back('C'); });
        gm.update();
        gm.update();
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
        POC_CHECK((order == std::vector<char> {'A', 'B', 'C', 'A', 'B'}));
    }

    void systemAddsOwnTypeDuringUpdate()
    {
        GameManager gm;
        int calls = 0;

        for (int i = 0; i < 4; i++)
            gm.addComponent<Health>(gm.createEntity(), 1);
        gm.createSystem<Health>([&gm, &calls](Health &h) {
            calls++;
            if (h.hp == 1)
                for (int i = 0; i < 100; i++)
                    gm.addComponent<Health>(gm.createEntity(), 2);
        });
        gm.update();
        POC_CHECK(gm.getArray<Health>().nbComponent() == 404);
        POC_CHECK(calls >= 4 && calls <= 404);
    }

    void systemDestroysCurrentEntity()
    {
        GameManager gm;

        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
        }
        gm.createSystem<Owned>([&gm](Owned &o) {
            if (o.value % 2 == 0)
                gm.destroyEntity(o.owner);
        });
        gm.update();
        POC_CHECK(gm.getArray<Owned>().nbComponent() == 50);
        for (const auto &pack : gm.getArray<Owned>().getDense())
            POC_CHECK(pack.component.value % 2 == 1);
    }

    void systemDestroysEveryEntity()
    {
        GameManager gm;
        int visits = 0;

        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
        }
        gm.createSystem<Owned>([&gm, &visits](Owned &o) {
            visits++;
            gm.destroyEntity(o.owner);
        });
        gm.update();
        POC_CHECK(visits == 100);
        POC_CHECK(gm.getArray<Owned>().nbComponent() == 0);
    }

    void systemDestroysItself()
    {
        GameManager gm;
        SystemId self = 0;
        int runs = 0;

        gm.addComponent<Health>(gm.createEntity(), 0);
        self = gm.createSystem<Health>([&](Health &) {
            runs++;
            gm.destroySystem(self);
        });
        gm.createSystem<Health>([&runs](Health &) { runs += 10; });
        gm.update();
        gm.update();
        POC_CHECK(runs == 21);
    }

    void throwingSystemDoesNotLockQueue()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool thrown = false;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([&thrown](Health &) {
            if (!thrown) {
                thrown = true;
                throw std::runtime_error("system failure");
            }
        });
        try {
            gm.update();
        } catch (const std::runtime_error &) {
        }
        gm.addComponent<Position>(e, 1.f, 1.f);
        POC_CHECK(gm.hasComponent<Position>(e));
    }

    void sameEntityDestroyedTwiceInOneUpdate()
    {
        GameManager gm;
        Entity enemy = gm.createEntity();
        Entity other = gm.createEntity();

        for (int i = 0; i < 2; i++) {
            Entity bullet = gm.createEntity();
            gm.addComponent<Owned>(bullet, enemy, i);
        }
        gm.createSystem<Owned>([&gm, other](Owned &o) {
            gm.destroyEntity(o.owner);
            if (o.value == 1)
                gm.addComponent<Health>(other, 7);
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(!gm.hasEntity(enemy));
        POC_CHECK(gm.hasComponent<Health>(other));
    }

    void particlesWithSystems()
    {
        struct Expected {
            float x0;
            float vx;
            int spawnTick;
            int lifetime;
        };

        GameManager gm;
        std::vector<std::pair<Entity, Expected>> particles;
        int moveVisits = 0;
        int ageVisits = 0;
        int destroyed = 0;

        gm.createSystem<Body>([&moveVisits](Body &b) {
            b.x += b.vx;
            b.y += b.vy;
            moveVisits++;
        });
        gm.createSystem<Lifetime>([&ageVisits](Lifetime &l) {
            l.ticks--;
            ageVisits++;
        });
        for (int tick = 0; tick < 300; tick++) {
            for (int i = 0; i < 5; i++) {
                Entity e = gm.createEntity();
                float vx = static_cast<float>((tick + i) % 7) - 3.f;
                int lifetime = 5 + (tick * 3 + i) % 40;
                gm.addComponent<Body>(e, static_cast<float>(i), 0.f, vx, 1.f);
                gm.addComponent<Lifetime>(e, e, lifetime);
                particles.emplace_back(
                    e, Expected {static_cast<float>(i), vx, tick, lifetime});
            }
            moveVisits = 0;
            ageVisits = 0;
            gm.update();
            POC_CHECK(moveVisits == static_cast<int>(particles.size()));
            POC_CHECK(ageVisits == static_cast<int>(particles.size()));
            std::vector<Entity> expired;
            for (const auto &pack : gm.getArray<Lifetime>().getDense())
                if (pack.component.ticks <= 0)
                    expired.push_back(pack.component.owner);
            for (Entity e : expired) {
                gm.destroyEntity(e);
                std::erase_if(
                    particles, [e](const auto &p) { return p.first == e; });
                destroyed++;
            }
            for (const auto &[e, p] : particles) {
                int age = tick - p.spawnTick + 1;
                const Body &b = gm.getComponent<Body>(e);
                POC_CHECK(b.x == p.x0 + p.vx * static_cast<float>(age));
                POC_CHECK(b.y == static_cast<float>(age));
                POC_CHECK(
                    gm.getComponent<Lifetime>(e).ticks == p.lifetime - age);
            }
            POC_CHECK(gm.getArray<Body>().nbComponent() == particles.size());
        }
        POC_CHECK(destroyed > 1000);
    }

} // namespace

namespace Poc {

    TestList systemTests()
    {
        return {
            {"update without systems does nothing", updateWithoutSystems},
            {"a system modifies the stored components",
                systemModifiesStoredComponents},
            {"a system does not copy the components",
                systemDoesNotCopyComponents},
            {"a system only touches its own type", systemOnlyTouchesItsType},
            {"a system visits each component exactly once",
                systemVisitsEachComponentOnce},
            {"systems run in creation order", systemsRunInCreationOrder},
            {"a later system sees the changes of an earlier one",
                laterSystemSeesEarlierChanges},
            {"a destroyed system stops running", destroyedSystemStopsRunning},
            {"order is kept after destroy then create",
                orderKeptAfterDestroyAndCreate},
            {"system handles stay valid after other destroys",
                handlesStayValidAfterOtherDestroys},
            {"a system created before any component of its type",
                systemCreatedBeforeAnyComponent},
            {"a system follows adds, removes and destroys",
                systemFollowsAddsAndRemoves},
            {"a system on an emptied array", systemOnEmptiedArray},
            {"1000 updates accumulate", repeatedUpdatesAccumulate},
            {"two systems on different types", twoSystemsOnDifferentTypes},
            {"a system creates entities of another type during update",
                systemAddsOtherTypeDuringUpdate},
            {"a system adds components with temporary arguments",
                systemAddsWithTemporaryArguments},
            {"a system modifies another type through an owner id",
                systemModifiesOtherTypeThroughOwner},
            {"a system destroys a later system during update",
                systemDestroysOtherSystem},
            {"[design] a system adds its own type during update",
                systemAddsOwnTypeDuringUpdate},
            {"[design] a system destroys the current entity during update",
                systemDestroysCurrentEntity},
            {"[design] a system destroys every entity during update",
                systemDestroysEveryEntity},
            {"[design] a system destroys itself during update",
                systemDestroysItself},
            {"a throwing system does not leave the queue locked",
                throwingSystemDoesNotLockQueue},
            {"[design] two bullets destroy the same enemy in one update",
                sameEntityDestroyedTwiceInOneUpdate},
            {"300 ticks of particles driven by systems", particlesWithSystems},
        };
    }

} // namespace Poc
