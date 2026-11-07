/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_systems
*/

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <criterion/criterion.h>

#include "Entity.hpp"
#include "GameManager.hpp"

namespace {

    struct Owned {
        RType::Entity owner;
        int value;
    };

    struct Position {
        float x;
        float y;
    };

    struct Velocity {
        float x;
        float y;
    };

    struct Health {
        int hp;
    };

    struct Name {
        std::string value;
    };

    struct Body {
        float x;
        float y;
        float vx;
        float vy;
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

    Test(systems, updateWithoutSystems,
        .description = "update without systems does nothing")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 5);
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(gm.getComponent<Health>(e).hp == 5);
    }

    Test(systems, systemModifiesStoredComponents,
        .description = "a system modifies the stored components")
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
            cr_assert(gm.getComponent<Health>(entities[i]).hp ==
                static_cast<int>(i) + 1000);
    }

    Test(systems, systemDoesNotCopyComponents,
        .description = "a system does not copy the components")
    {
        Tracked::alive = 0;
        GameManager gm;
        int seenAlive = -1;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Tracked>(gm.createEntity(), i);
        gm.createSystem<Tracked>(
            [&seenAlive](Tracked &) { seenAlive = Tracked::alive; });
        gm.update();
        cr_assert(seenAlive == 3);
        cr_assert(Tracked::alive == 3);
    }

    Test(systems, systemOnlyTouchesItsType,
        .description = "a system only touches its own type")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        gm.addComponent<Velocity>(e, 1.f, 1.f);
        gm.createSystem<Position>([](Position &p) { p.x = 42.f; });
        gm.update();
        cr_assert(gm.getComponent<Position>(e).x == 42.f);
        cr_assert(gm.getComponent<Velocity>(e).x == 1.f);
    }

    Test(systems, systemVisitsEachComponentOnce,
        .description = "a system visits each component exactly once")
    {
        GameManager gm;
        std::map<int, int> visits;

        for (int i = 0; i < 500; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<Health>([&visits](Health &h) { visits[h.hp]++; });
        gm.update();
        cr_assert(visits.size() == 500);
        for (const auto &[_, count] : visits)
            cr_assert(count == 1);
    }

    Test(systems, systemsRunInCreationOrder,
        .description = "systems run in creation order")
    {
        GameManager gm;
        std::vector<int> order;

        gm.addComponent<Health>(gm.createEntity(), 0);
        gm.createSystem<Health>([&order](Health &) { order.push_back(1); });
        gm.createSystem<Health>([&order](Health &) { order.push_back(2); });
        gm.createSystem<Health>([&order](Health &) { order.push_back(3); });
        gm.update();
        gm.update();
        cr_assert((order == std::vector<int> {1, 2, 3, 1, 2, 3}));
    }

    Test(systems, laterSystemSeesEarlierChanges,
        .description = "a later system sees the changes of an earlier one")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int seen = -1;

        gm.addComponent<Health>(e, 0);
        gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        gm.createSystem<Health>([&seen](Health &h) { seen = h.hp; });
        gm.update();
        cr_assert(seen == 1);
        gm.update();
        cr_assert(seen == 2);
    }

    Test(systems, destroyedSystemStopsRunning,
        .description = "a destroyed system stops running")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 0);
        auto sys = gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        gm.update();
        gm.destroySystem(sys);
        gm.update();
        cr_assert(gm.getComponent<Health>(e).hp == 1);
    }

    Test(systems, orderKeptAfterDestroyAndCreate,
        .description = "order is kept after destroy then create")
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
        cr_assert((order == std::vector<char> {'A', 'C', 'D'}));
    }

    Test(systems, handlesStayValidAfterOtherDestroys,
        .description = "system handles stay valid after other destroys")
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
        cr_assert((order == std::vector<char> {'C', 'E'}));
    }

    Test(systems, systemCreatedBeforeAnyComponent,
        .description = "a system created before any component of its type")
    {
        GameManager gm;
        int visits = 0;

        gm.createSystem<Health>([&visits](Health &) { visits++; });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(visits == 0);
        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.update();
        cr_assert(visits == 1);
    }

    Test(systems, systemFollowsAddsAndRemoves,
        .description = "a system follows adds, removes and destroys")
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
        cr_assert(visits == 10);
        visits = 0;
        gm.removeComponent<Health>(entities[0]);
        gm.destroyEntity(entities[5]);
        entities.erase(entities.begin() + 5);
        gm.addComponent<Health>(gm.createEntity(), 99);
        gm.update();
        cr_assert(visits == 9);
        visits = 0;
        for (Entity e : entities)
            if (gm.hasEntity(e))
                gm.destroyEntity(e);
        gm.update();
        cr_assert(visits == 1);
    }

    Test(systems, systemOnEmptiedArray,
        .description = "a system on an emptied array")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int visits = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([&visits](Health &) { visits++; });
        gm.removeComponent<Health>(e);
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(visits == 0);
    }

    Test(systems, repeatedUpdatesAccumulate,
        .description = "1000 updates accumulate")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 0);
        gm.createSystem<Health>([](Health &h) { h.hp += 1; });
        for (int i = 0; i < 1000; i++)
            gm.update();
        cr_assert(gm.getComponent<Health>(e).hp == 1000);
    }

    Test(systems, twoSystemsOnDifferentTypes,
        .description = "two systems on different types")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 0.f, 0.f);
        gm.addComponent<Health>(e, 10);
        gm.createSystem<Position>([](Position &p) { p.x += 1.f; });
        gm.createSystem<Health>([](Health &h) { h.hp -= 1; });
        gm.update();
        gm.update();
        cr_assert(gm.getComponent<Position>(e).x == 2.f);
        cr_assert(gm.getComponent<Health>(e).hp == 8);
    }

    Test(systems, systemAddsOtherTypeDuringUpdate,
        .description =
            "a system creates entities of another type during update")
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
        cr_assert(spawned.size() == 20);
        cr_assert(gm.getArray<Health>().nbComponent() == 20);
        for (auto &s : spawned)
            cr_assert(gm.hasComponent<Health>(s));
    }

    Test(systems, systemAddsWithTemporaryArguments,
        .description = "a system adds components with temporary arguments")
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
        cr_assert(spawned.size() == 20);
        for (const auto &[e, value] : spawned) {
            cr_assert(gm.getComponent<Name>(e).value ==
                std::string(64, 'x') + std::to_string(value));
            cr_assert(gm.getComponent<Health>(e).hp == value * 10);
        }
    }

    Test(systems, systemModifiesOtherTypeThroughOwner,
        .description = "a system modifies another type through an owner id")
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
            cr_assert(gm.getComponent<Health>(entities[i]).hp ==
                static_cast<int>(i) * 2);
    }

    Test(systems, systemDestroysOtherSystem,
        .description = "a system destroys a later system during update")
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
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
        cr_assert((order == std::vector<char> {'A', 'B', 'C', 'A', 'B'}));
    }

    Test(systems, systemAddsOwnTypeDuringUpdate,
        .description = "[design] a system adds its own type during update")
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
        cr_assert(gm.getArray<Health>().nbComponent() == 404);
        cr_assert(calls >= 4 && calls <= 404);
    }

    Test(systems, systemDestroysCurrentEntity,
        .description =
            "[design] a system destroys the current entity during update")
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
        cr_assert(gm.getArray<Owned>().nbComponent() == 50);
        for (const auto &pack : gm.getArray<Owned>().getDense())
            cr_assert(pack.component.value % 2 == 1);
    }

    Test(systems, systemDestroysEveryEntity,
        .description = "[design] a system destroys every entity during update")
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
        cr_assert(visits == 100);
        cr_assert(gm.getArray<Owned>().nbComponent() == 0);
    }

    Test(systems, systemDestroysItself,
        .description = "[design] a system destroys itself during update")
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
        cr_assert(runs == 21);
    }

    Test(systems, throwingSystemDoesNotLockQueue,
        .description = "a throwing system does not leave the queue locked")
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
        cr_assert(gm.hasComponent<Position>(e));
    }

    Test(systems, sameEntityDestroyedTwiceInOneUpdate,
        .description =
            "[design] two bullets destroy the same enemy in one update")
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
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(!gm.hasEntity(enemy));
        cr_assert(gm.hasComponent<Health>(other));
    }

    Test(systems, particlesWithSystems,
        .description = "300 ticks of particles driven by systems")
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
            cr_assert(moveVisits == static_cast<int>(particles.size()));
            cr_assert(ageVisits == static_cast<int>(particles.size()));
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
                cr_assert(b.x == p.x0 + p.vx * static_cast<float>(age));
                cr_assert(b.y == static_cast<float>(age));
                cr_assert(
                    gm.getComponent<Lifetime>(e).ticks == p.lifetime - age);
            }
            cr_assert(gm.getArray<Body>().nbComponent() == particles.size());
        }
        cr_assert(destroyed > 1000);
    }

} // namespace
