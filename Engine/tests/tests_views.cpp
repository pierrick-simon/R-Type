/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_views
*/

#include <algorithm>
#include <cstddef>
#include <functional>
#include <random>
#include <type_traits>
#include <vector>

#include <criterion/criterion.h>
#include "ComponentManager.hpp"

#include "Entity.hpp"
#include "GameManager.hpp"
#include "View.hpp"

// Expected API (steps 1 to 3):
//   RType::View<Ts...> view(componentManager);
//   view.each([](RType::Entity e, Ts &...components) { ... });
// A view on a type that has never been added is empty and never throws.

namespace {

    struct Player {};

    struct Enemy {};

    struct Bullet {};

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

    using RType::ComponentManager;
    using RType::Entity;
    using RType::EntityManager;
    using RType::GameManager;
    using RType::View;

    Test(views, viewOnNeverAddedTypeIsEmpty,
        .description = "[view] a view on a never added type is empty")
    {
        ComponentManager cm;
        int calls = 0;

        cr_assert_no_throw(static_cast<void>(View<Health>(cm)), std::exception);
        View<Health> view(cm);
        cr_assert_no_throw(
            view.each([&calls](Entity, Health &) { calls++; }), std::exception);
        cr_assert(calls == 0);
    }

    Test(views, viewWithOneMissingTypeIsEmpty,
        .description = "[view] a view with one missing type is empty")
    {
        ComponentManager cm;
        EntityManager em;
        int calls = 0;

        cm.addComponent<Health>(em.createEntity(), 1);
        View<Health, Damage> view(cm);
        cr_assert_no_throw(
            view.each([&calls](Entity, Health &, Damage &) { calls++; }),
            std::exception);
        cr_assert(calls == 0);
    }

    Test(views, viewOnASingleType,
        .description = "[view] a view on a single type")
    {
        ComponentManager cm;
        EntityManager em;
        EntitySet expected;
        EntitySet visited;

        for (int i = 0; i < 10; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Health>(e, i);
            expected.insert(e);
        }
        View<Health>(cm).each(
            [&visited](Entity e, Health &) { visited.insert(e); });
        cr_assert(visited.size() == expected.size());
        for (Entity e : expected)
            cr_assert(visited.contains(e));
    }

    Test(views, viewVisitsOnlyEntitiesWithAllTypes,
        .description = "[view] visits only entities that have all the types")
    {
        ComponentManager cm;
        EntityManager em;
        EntitySet expected;
        EntitySet visited;

        for (int i = 0; i < 30; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Health>(e, i);
            if (i % 3 != 0) {
                cm.addComponent<Damage>(e, i);
                expected.insert(e);
            }
        }
        View<Health, Damage>(cm).each(
            [&visited](Entity e, Health &, Damage &) { visited.insert(e); });
        cr_assert(visited.size() == expected.size());
        for (Entity e : expected)
            cr_assert(visited.contains(e));
    }

    Test(views, viewGivesTheRightEntity,
        .description = "[view] gives the right entity")
    {
        ComponentManager cm;
        EntityManager em;
        bool ok = true;
        int calls = 0;

        for (int i = 0; i < 50; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Health>(e, i);
            cm.addComponent<Damage>(e, i * 2);
        }
        View<Health, Damage>(cm).each(
            [&cm, &ok, &calls](Entity e, Health &h, Damage &d) {
                calls++;
                if (&cm.getComponent<Health>(e) != &h ||
                    &cm.getComponent<Damage>(e) != &d || d.value != h.hp * 2)
                    ok = false;
            });
        cr_assert(calls == 50);
        cr_assert(ok);
    }

    Test(views, viewGivesTheCurrentGeneration,
        .description = "[view] gives the current generation of a recycled slot")
    {
        ComponentManager cm;
        EntityManager em;
        std::vector<Entity> seen;

        Entity old = em.createEntity();
        cm.addComponent<Health>(old, 0);
        cm.removeComponents(old);
        em.destroyEntity(old);
        Entity reused = em.createEntity();
        cr_assert(indexOf(reused) == indexOf(old));
        cm.addComponent<Health>(reused, 1);
        View<Health>(cm).each(
            [&seen](Entity e, Health &) { seen.push_back(e); });
        cr_assert(seen.size() == 1);
        cr_assert(seen[0] == reused);
        cr_assert(!(seen[0] == old));
    }

    Test(views, viewModificationsReachTheStoredComponents,
        .description = "[view] modifications reach the stored components")
    {
        ComponentManager cm;
        EntityManager em;
        Entity e = em.createEntity();

        cm.addComponent<Position>(e, 0.f, 0.f);
        cm.addComponent<Velocity>(e, 2.f, 3.f);
        View<Position, Velocity>(cm).each([](Entity, Position &p, Velocity &v) {
            p.x += v.x;
            p.y += v.y;
        });
        cr_assert(cm.getComponent<Position>(e).x == 2.f);
        cr_assert(cm.getComponent<Position>(e).y == 3.f);
    }

    Test(views, viewArgumentsFollowTheTemplateOrder,
        .description = "[view] arguments follow the template order")
    {
        ComponentManager cm;
        EntityManager em;
        Entity e = em.createEntity();
        bool ok = false;

        cm.addComponent<Health>(e, 7);
        cm.addComponent<Damage>(e, 3);
        View<Damage, Health>(cm).each([&ok](Entity, Damage &d, Health &h) {
            ok = d.value == 3 && h.hp == 7;
        });
        cr_assert(ok);
    }

    Test(views, viewTypeOrderDoesNotChangeTheResult,
        .description = "[view] the type order does not change the result")
    {
        ComponentManager cm;
        EntityManager em;
        EntitySet rareFirst;
        EntitySet commonFirst;

        for (int i = 0; i < 100; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Health>(e, i);
            if (i % 25 == 0)
                cm.addComponent<Player>(e);
        }
        View<Player, Health>(cm).each(
            [&rareFirst](
                Entity e, Player &, Health &) { rareFirst.insert(e); });
        View<Health, Player>(cm).each(
            [&commonFirst](
                Entity e, Health &, Player &) { commonFirst.insert(e); });
        cr_assert(rareFirst.size() == 4);
        cr_assert(commonFirst.size() == 4);
        for (Entity e : rareFirst)
            cr_assert(commonFirst.contains(e));
    }

    Test(views, viewAcceptsAGenericLambda,
        .description = "[view] accepts a generic lambda")
    {
        ComponentManager cm;
        EntityManager em;
        int calls = 0;

        for (int i = 0; i < 5; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Health>(e, i);
            cm.addComponent<Damage>(e, i);
        }
        View<Health, Damage>(cm).each([&calls](auto, auto &...) { calls++; });
        cr_assert(calls == 5);
    }

    Test(views, viewAcceptsConstParameters,
        .description = "[view] accepts const parameters")
    {
        ComponentManager cm;
        EntityManager em;
        Entity e = em.createEntity();
        int seen = 0;

        cm.addComponent<Health>(e, 4);
        View<Health>(cm).each(
            [&seen](const Entity &, const Health &h) { seen = h.hp; });
        cr_assert(seen == 4);
    }

    Test(views, viewDoesNotCopyComponents,
        .description = "[view] does not copy the components")
    {
        Tracked::alive = 0;
        ComponentManager cm;
        EntityManager em;
        int seenAlive = -1;

        for (int i = 0; i < 3; i++) {
            Entity e = em.createEntity();
            cm.addComponent<Tracked>(e, i);
            cm.addComponent<Health>(e, i);
        }
        View<Tracked, Health>(cm).each(
            [&seenAlive](
                Entity, Tracked &, Health &) { seenAlive = Tracked::alive; });
        cr_assert(seenAlive == 3);
        cr_assert(Tracked::alive == 3);
    }

    Test(views, viewSeesComponentsAddedAfterItsCreation,
        .description = "[view] sees components added after its creation")
    {
        ComponentManager cm;
        EntityManager em;
        int calls = 0;

        cm.addComponent<Health>(em.createEntity(), 0);
        View<Health> view(cm);
        cm.addComponent<Health>(em.createEntity(), 1);
        view.each([&calls](Entity, Health &) { calls++; });
        cr_assert(calls == 2);
    }

    Test(views, viewOnThreeTypesMatchesAModel,
        .description = "[view] 1000 entities on three types match a model")
    {
        ComponentManager cm;
        EntityManager em;
        std::mt19937 rng(99);
        std::vector<Entity> entities;
        std::vector<bool> match;
        EntitySet visited;

        for (int i = 0; i < 1000; i++) {
            Entity e = em.createEntity();
            bool health = rng() % 2 == 0;
            bool damage = rng() % 3 == 0;
            bool player = rng() % 4 == 0;
            if (health)
                cm.addComponent<Health>(e, i);
            if (damage)
                cm.addComponent<Damage>(e, i);
            if (player)
                cm.addComponent<Player>(e);
            entities.push_back(e);
            match.push_back(health && damage && player);
        }
        View<Health, Damage, Player>(cm).each(
            [&visited](Entity e, Health &h, Damage &d, Player &) {
                cr_assert(h.hp == d.value);
                visited.insert(e);
            });
        std::size_t expected = 0;
        for (std::size_t i = 0; i < entities.size(); i++) {
            expected += match[i] ? 1 : 0;
            cr_assert(visited.contains(entities[i]) == match[i]);
        }
        cr_assert(visited.size() == expected);
    }

    // Step 5: through GameManager, gm.view<Ts...>() returning a View.

    // Runs func once per update(), in system order, until per-frame
    // systems exist (step 7): a system on a tag that a single entity has.
    template<int N>
    struct FrameTag {};

    template<int N>
    void createFrameSystem(GameManager &gm, std::function<void()> func)
    {
        gm.addComponent<FrameTag<N>>(gm.createEntity());
        gm.createSystem<FrameTag<N>>(
            [func = std::move(func)](FrameTag<N> &) { func(); });
    }

    Test(views, gmViewOnNeverAddedTypeIsEmpty,
        .description = "[view] gm.view on a never added type is empty")
    {
        GameManager gm;
        int calls = 0;

        gm.createEntity();
        // Parenthesized: the comma in view<Health, Damage> splits macro args.
        cr_assert_no_throw(
            (gm.view<Health, Damage>().each(
                [&calls](Entity, Health &, Damage &) { calls++; })),
            std::exception);
        cr_assert(calls == 0);
    }

    Test(views, gmViewGivesLiveEntitiesWithAllTypes,
        .description =
            "[view] gm.view gives live entities that have all the types")
    {
        GameManager gm;
        EntitySet expected;
        EntitySet visited;
        bool ok = true;

        for (int i = 0; i < 40; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            if (i % 4 != 0) {
                gm.addComponent<Damage>(e, i);
                expected.insert(e);
            }
        }
        gm.view<Damage, Health>().each(
            [&gm, &visited, &ok](Entity e, Damage &d, Health &h) {
                visited.insert(e);
                if (!gm.hasEntity(e) || &gm.getComponent<Health>(e) != &h ||
                    &gm.getComponent<Damage>(e) != &d)
                    ok = false;
            });
        cr_assert(ok);
        cr_assert(visited.size() == expected.size());
        for (Entity e : expected)
            cr_assert(visited.contains(e));
    }

    Test(views, gmViewSkipsDestroyedEntities,
        .description = "[view] gm.view skips destroyed entities")
    {
        GameManager gm;
        std::vector<Entity> entities;
        EntitySet visited;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
        }
        for (std::size_t i = 0; i < entities.size(); i += 2)
            gm.destroyEntity(entities[i]);
        for (int i = 0; i < 5; i++)
            gm.addComponent<Health>(gm.createEntity(), 100 + i);
        gm.view<Health>().each(
            [&visited](Entity e, Health &) { visited.insert(e); });
        cr_assert(visited.size() == 15);
        for (std::size_t i = 0; i < entities.size(); i += 2)
            cr_assert(!visited.contains(entities[i]));
        for (Entity e : visited)
            cr_assert(gm.hasEntity(e));
    }

    Test(views, gmViewEntityCanBeDestroyedInASystem,
        .description =
            "[view] an entity from gm.view can be destroyed in a system")
    {
        GameManager gm;
        int seenInLaterSystem = 0;

        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i < 4 ? 0 : 10);
        }
        createFrameSystem<0>(gm, [&gm] {
            gm.view<Health>().each([&gm](Entity e, Health &h) {
                if (h.hp <= 0)
                    gm.destroyEntity(e);
            });
        });
        createFrameSystem<1>(gm, [&gm, &seenInLaterSystem] {
            gm.view<Health>().each([&seenInLaterSystem](Entity, Health &) {
                seenInLaterSystem++;
            });
        });
        gm.update();
        cr_assert(seenInLaterSystem == 10);
        cr_assert(gm.getArray<Health>().nbComponent() == 6);
        int left = 0;
        gm.view<Health>().each([&left](Entity, Health &h) {
            cr_assert(h.hp == 10);
            left++;
        });
        cr_assert(left == 6);
    }

    Test(views, gmViewAddDuringUpdateIsVisibleNextFrame,
        .description =
            "[view] a component added during update is visible next frame")
    {
        GameManager gm;
        int damaged = 0;

        for (int i = 0; i < 5; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        createFrameSystem<0>(gm, [&gm, &damaged] {
            gm.view<Health>().each([&gm](Entity e, Health &) {
                if (!gm.hasComponent<Damage>(e))
                    gm.addComponent<Damage>(e, 1);
            });
            damaged = 0;
            gm.view<Health, Damage>().each(
                [&damaged](Entity, Health &, Damage &) { damaged++; });
        });
        gm.update();
        cr_assert(damaged == 0);
        gm.update();
        cr_assert(damaged == 5);
    }

    Test(views, gmViewBulletsHitEnemies,
        .description = "[view] bullets hit enemies through two nested views")
    {
        GameManager gm;
        std::vector<Entity> enemies;

        for (int i = 0; i < 3; i++) {
            Entity enemy = gm.createEntity();
            gm.addComponent<Enemy>(enemy);
            gm.addComponent<Position>(enemy, static_cast<float>(i * 10), 0.f);
            gm.addComponent<Health>(enemy, 3);
            enemies.push_back(enemy);
        }
        for (float x : {0.f, 0.f, 10.f, 55.f}) {
            Entity bullet = gm.createEntity();
            gm.addComponent<Bullet>(bullet);
            gm.addComponent<Position>(bullet, x, 0.f);
            gm.addComponent<Damage>(bullet, 1);
        }
        createFrameSystem<0>(gm, [&gm] {
            gm.view<Bullet, Position, Damage>().each(
                [&gm](Entity bullet, Bullet &, Position &bp, Damage &d) {
                    gm.view<Enemy, Position, Health>().each(
                        [&](Entity, Enemy &, Position &ep, Health &h) {
                            if (bp.x == ep.x) {
                                h.hp -= d.value;
                                gm.destroyEntity(bullet);
                            }
                        });
                });
        });
        gm.update();
        cr_assert(gm.getComponent<Health>(enemies[0]).hp == 1);
        cr_assert(gm.getComponent<Health>(enemies[1]).hp == 2);
        cr_assert(gm.getComponent<Health>(enemies[2]).hp == 3);
        cr_assert(gm.getArray<Bullet>().nbComponent() == 1);
    }

} // namespace
