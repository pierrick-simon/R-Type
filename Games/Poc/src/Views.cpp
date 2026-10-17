/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Views
*/

#include <cstddef>
#include <functional>
#include <random>
#include <vector>

#include "ComponentManager.hpp"
#include "Components.hpp"
#include "Entity.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"
#include "View.hpp"

// Expected API (steps 1 to 3):
//   RType::View<Ts...> view(componentManager);
//   view.each([](RType::Entity e, Ts &...components) { ... });
// A view on a type that has never been added is empty and never throws.

namespace {

    using RType::ComponentManager;
    using RType::Entity;
    using RType::EntityManager;
    using RType::GameManager;
    using RType::View;
    using namespace Poc;

    void viewOnNeverAddedTypeIsEmpty()
    {
        ComponentManager cm;
        int calls = 0;

        POC_CHECK_NOTHROW(View<Health>(cm));
        View<Health> view(cm);
        POC_CHECK_NOTHROW(view.each([&calls](Entity, Health &) { calls++; }));
        POC_CHECK(calls == 0);
    }

    void viewWithOneMissingTypeIsEmpty()
    {
        ComponentManager cm;
        EntityManager em;
        int calls = 0;

        cm.addComponent<Health>(em.createEntity(), 1);
        View<Health, Damage> view(cm);
        POC_CHECK_NOTHROW(
            view.each([&calls](Entity, Health &, Damage &) { calls++; }));
        POC_CHECK(calls == 0);
    }

    void viewOnASingleType()
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
        POC_CHECK(visited.size() == expected.size());
        for (Entity e : expected)
            POC_CHECK(visited.contains(e));
    }

    void viewVisitsOnlyEntitiesWithAllTypes()
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
        POC_CHECK(visited.size() == expected.size());
        for (Entity e : expected)
            POC_CHECK(visited.contains(e));
    }

    void viewGivesTheRightEntity()
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
        POC_CHECK(calls == 50);
        POC_CHECK(ok);
    }

    void viewGivesTheCurrentGeneration()
    {
        ComponentManager cm;
        EntityManager em;
        std::vector<Entity> seen;

        Entity old = em.createEntity();
        cm.addComponent<Health>(old, 0);
        cm.removeComponents(old);
        em.destroyEntity(old);
        Entity reused = em.createEntity();
        POC_CHECK(indexOf(reused) == indexOf(old));
        cm.addComponent<Health>(reused, 1);
        View<Health>(cm).each(
            [&seen](Entity e, Health &) { seen.push_back(e); });
        POC_CHECK(seen.size() == 1);
        POC_CHECK(seen[0] == reused);
        POC_CHECK(!(seen[0] == old));
    }

    void viewModificationsReachTheStoredComponents()
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
        POC_CHECK(cm.getComponent<Position>(e).x == 2.f);
        POC_CHECK(cm.getComponent<Position>(e).y == 3.f);
    }

    void viewArgumentsFollowTheTemplateOrder()
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
        POC_CHECK(ok);
    }

    void viewTypeOrderDoesNotChangeTheResult()
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
        POC_CHECK(rareFirst.size() == 4);
        POC_CHECK(commonFirst.size() == 4);
        for (Entity e : rareFirst)
            POC_CHECK(commonFirst.contains(e));
    }

    void viewAcceptsAGenericLambda()
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
        POC_CHECK(calls == 5);
    }

    void viewAcceptsConstParameters()
    {
        ComponentManager cm;
        EntityManager em;
        Entity e = em.createEntity();
        int seen = 0;

        cm.addComponent<Health>(e, 4);
        View<Health>(cm).each(
            [&seen](const Entity &, const Health &h) { seen = h.hp; });
        POC_CHECK(seen == 4);
    }

    void viewDoesNotCopyComponents()
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
        POC_CHECK(seenAlive == 3);
        POC_CHECK(Tracked::alive == 3);
    }

    void viewSeesComponentsAddedAfterItsCreation()
    {
        ComponentManager cm;
        EntityManager em;
        int calls = 0;

        cm.addComponent<Health>(em.createEntity(), 0);
        View<Health> view(cm);
        cm.addComponent<Health>(em.createEntity(), 1);
        view.each([&calls](Entity, Health &) { calls++; });
        POC_CHECK(calls == 2);
    }

    void viewOnThreeTypesMatchesAModel()
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
                POC_CHECK(h.hp == d.value);
                visited.insert(e);
            });
        std::size_t expected = 0;
        for (std::size_t i = 0; i < entities.size(); i++) {
            expected += match[i] ? 1 : 0;
            POC_CHECK(visited.contains(entities[i]) == match[i]);
        }
        POC_CHECK(visited.size() == expected);
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

    void gmViewOnNeverAddedTypeIsEmpty()
    {
        GameManager gm;
        int calls = 0;

        gm.createEntity();
        // Parenthesized: the comma in view<Health, Damage> splits macro args.
        POC_CHECK_NOTHROW((gm.view<Health, Damage>().each(
            [&calls](Entity, Health &, Damage &) { calls++; })));
        POC_CHECK(calls == 0);
    }

    void gmViewGivesLiveEntitiesWithAllTypes()
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
        POC_CHECK(ok);
        POC_CHECK(visited.size() == expected.size());
        for (Entity e : expected)
            POC_CHECK(visited.contains(e));
    }

    void gmViewSkipsDestroyedEntities()
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
        POC_CHECK(visited.size() == 15);
        for (std::size_t i = 0; i < entities.size(); i += 2)
            POC_CHECK(!visited.contains(entities[i]));
        for (Entity e : visited)
            POC_CHECK(gm.hasEntity(e));
    }

    void gmViewEntityCanBeDestroyedInASystem()
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
        POC_CHECK(seenInLaterSystem == 10);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 6);
        int left = 0;
        gm.view<Health>().each([&left](Entity, Health &h) {
            POC_CHECK(h.hp == 10);
            left++;
        });
        POC_CHECK(left == 6);
    }

    void gmViewAddDuringUpdateIsVisibleNextFrame()
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
        POC_CHECK(damaged == 0);
        gm.update();
        POC_CHECK(damaged == 5);
    }

    void gmViewBulletsHitEnemies()
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
        POC_CHECK(gm.getComponent<Health>(enemies[0]).hp == 1);
        POC_CHECK(gm.getComponent<Health>(enemies[1]).hp == 2);
        POC_CHECK(gm.getComponent<Health>(enemies[2]).hp == 3);
        POC_CHECK(gm.getArray<Bullet>().nbComponent() == 1);
    }

} // namespace

namespace Poc {

    TestList viewTests()
    {
        return {
            {"[view] a view on a never added type is empty",
                viewOnNeverAddedTypeIsEmpty},
            {"[view] a view with one missing type is empty",
                viewWithOneMissingTypeIsEmpty},
            {"[view] a view on a single type", viewOnASingleType},
            {"[view] visits only entities that have all the types",
                viewVisitsOnlyEntitiesWithAllTypes},
            {"[view] gives the right entity", viewGivesTheRightEntity},
            {"[view] gives the current generation of a recycled slot",
                viewGivesTheCurrentGeneration},
            {"[view] modifications reach the stored components",
                viewModificationsReachTheStoredComponents},
            {"[view] arguments follow the template order",
                viewArgumentsFollowTheTemplateOrder},
            {"[view] the type order does not change the result",
                viewTypeOrderDoesNotChangeTheResult},
            {"[view] accepts a generic lambda", viewAcceptsAGenericLambda},
            {"[view] accepts const parameters", viewAcceptsConstParameters},
            {"[view] does not copy the components", viewDoesNotCopyComponents},
            {"[view] sees components added after its creation",
                viewSeesComponentsAddedAfterItsCreation},
            {"[view] 1000 entities on three types match a model",
                viewOnThreeTypesMatchesAModel},
            {"[view] gm.view on a never added type is empty",
                gmViewOnNeverAddedTypeIsEmpty},
            {"[view] gm.view gives live entities that have all the types",
                gmViewGivesLiveEntitiesWithAllTypes},
            {"[view] gm.view skips destroyed entities",
                gmViewSkipsDestroyedEntities},
            {"[view] an entity from gm.view can be destroyed in a system",
                gmViewEntityCanBeDestroyedInASystem},
            {"[view] a component added during update is visible next frame",
                gmViewAddDuringUpdateIsVisibleNextFrame},
            {"[view] bullets hit enemies through two nested views",
                gmViewBulletsHitEnemies},
        };
    }

} // namespace Poc
