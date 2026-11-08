/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_multi_systems
*/

#include <algorithm>
#include <cstddef>
#include <random>
#include <type_traits>
#include <vector>

#include <criterion/criterion.h>

#include "Entity.hpp"
#include "GameManager.hpp"

namespace {

    struct Player {};

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

    struct Damage {
        int value;
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

    template<typename E>
    std::size_t indexOf(const E &entity)
    {
        if constexpr (std::is_integral_v<E>)
            return static_cast<std::size_t>(entity);
        else if constexpr (requires { entity.index(); })
            return static_cast<std::size_t>(entity.index());
        else
            return static_cast<std::size_t>(entity.index);
    }

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

    // Owned identifies the visited entity inside a callback (owner field).

    Test(multi_systems, visitsOnlyEntitiesWithAllTypes,
        .description = "[multi] visits only entities that have all the types")
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
        cr_assert(visited.size() == expected.size());
        for (Entity e : expected)
            cr_assert(visited.contains(e));
    }

    Test(multi_systems, firstEntityMissingATypeDoesNotStopTheSystem,
        .description =
            "[multi] a first entity missing a type does not stop the system")
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
        cr_assert(calls == 5);
    }

    Test(multi_systems, entityMissingATypeInTheMiddle,
        .description =
            "[multi] an entity missing a type in the middle is skipped")
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
        cr_assert(calls == 8);
    }

    Test(multi_systems, componentsBelongToTheSameEntity,
        .description = "[multi] the components belong to the same entity")
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
        cr_assert(calls == 200);
        cr_assert(paired);
    }

    Test(multi_systems, pairingSurvivesRemovalsAndRecycling,
        .description = "[multi] pairing survives removals and recycling")
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
        cr_assert(calls == 80);
        cr_assert(paired);
    }

    Test(multi_systems, modificationsReachTheStoredComponents,
        .description = "[multi] modifications reach the stored components")
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
        cr_assert(gm.getComponent<Position>(e).x == 5.f);
        cr_assert(gm.getComponent<Position>(e).y == 6.f);
        cr_assert(gm.getComponent<Velocity>(e).x == 4.f);
    }

    Test(multi_systems, argumentsFollowTheTemplateOrder,
        .description = "[multi] arguments follow the template order")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool ok = false;

        gm.addComponent<Health>(e, 7);
        gm.addComponent<Damage>(e, 3);
        gm.createSystem<Damage, Health>(
            [&ok](Damage &d, Health &h) { ok = d.value == 3 && h.hp == 7; });
        gm.update();
        cr_assert(ok);
    }

    Test(multi_systems, typeOrderDoesNotChangeTheResult,
        .description = "[multi] the type order does not change the result")
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
        cr_assert(rareFirst.size() == 4);
        cr_assert(commonFirst.size() == 4);
        for (Entity e : rareFirst)
            cr_assert(commonFirst.contains(e));
    }

    Test(multi_systems, threeTypes,
        .description = "[multi] a system on three types")
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
                cr_assert(o.value % 6 == 0);
                cr_assert(h.hp == o.value && d.value == o.value);
            });
        gm.update();
        cr_assert(calls == 5);
    }

    Test(multi_systems, typeNeverAddedDoesNothing,
        .description = "[multi] a type never added does nothing")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 0);
        gm.addComponent<Damage>(e, 1);
        gm.update();
        cr_assert(calls == 1);
    }

    Test(multi_systems, emptiedFirstArrayDoesNothing,
        .description = "[multi] an emptied first array does nothing")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.addComponent<Damage>(e, 1);
        gm.removeComponent<Health>(e);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 0);
    }

    Test(multi_systems, multiSystemDoesNotCopyComponents,
        .description = "[multi] a system does not copy the components")
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
        cr_assert(seenAlive == 3);
        cr_assert(Tracked::alive == 3);
    }

    Test(multi_systems, constParameters,
        .description = "[multi] const parameters")
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
        cr_assert(seen == 5.f);
    }

    Test(multi_systems, followsAddsAndRemovesAcrossFrames,
        .description = "[multi] follows adds and removes across frames")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        int calls = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health, Damage>(
            [&calls](Health &, Damage &) { calls++; });
        gm.addComponent<Damage>(gm.createEntity(), 0);
        gm.update();
        cr_assert(calls == 0);
        gm.addComponent<Damage>(e, 1);
        gm.update();
        cr_assert(calls == 1);
        gm.removeComponent<Health>(e);
        gm.update();
        cr_assert(calls == 1);
        gm.addComponent<Health>(e, 2);
        gm.update();
        cr_assert(calls == 2);
    }

    Test(multi_systems, removalDuringUpdateIsDeferred,
        .description = "[multi] a removal during update is deferred")
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
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(calls == 10);
        cr_assert(gm.getArray<Health>().nbComponent() == 0);
        gm.update();
        cr_assert(calls == 10);
    }

    Test(multi_systems, destroyDuringUpdateStaysVisibleThisFrame,
        .description =
            "[multi] a destroyed entity stays visible until the frame ends")
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
        cr_assert(firstCalls == 10);
        cr_assert(secondCalls == 10);
        gm.update();
        cr_assert(firstCalls == 10);
        cr_assert(secondCalls == 10);
    }

    Test(multi_systems, recycledSlotDoesNotInheritComponents,
        .description = "[multi] a recycled slot does not inherit components")
    {
        GameManager gm;
        int calls = 0;

        Entity old = gm.createEntity();
        gm.addComponent<Owned>(old, old, 0);
        gm.addComponent<Health>(old, 0);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        cr_assert(indexOf(reused) == indexOf(old));
        gm.addComponent<Owned>(reused, reused, 1);
        gm.createSystem<Owned, Health>(
            [&calls](Owned &, Health &) { calls++; });
        gm.update();
        cr_assert(calls == 0);
    }

    Test(multi_systems, denseKnowsTheOwnerOfEachComponent,
        .description = "[multi] the dense knows the owner of each component")
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
            cr_assert(gm.hasEntity(pack.entity()));
            cr_assert(
                &gm.getComponent<Health>(pack.entity()) == &pack.component);
        }
        cr_assert(count == gm.getArray<Health>().nbComponent());
    }

    Test(multi_systems, randomSubsetsMatchModel,
        .description = "[multi] 1000 random subsets match a model")
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
            cr_assert(visited.contains(entities[i]) == match);
        }
        cr_assert(visited.size() == expected);
    }

} // namespace
