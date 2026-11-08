/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_view_vectors
*/

#include <algorithm>
#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include <criterion/criterion.h>
#include "ComponentManager.hpp"

#include "Entity.hpp"
#include "GameManager.hpp"
#include "View.hpp"

// Expected API: a view can be turned into a vector holding, for each
// matching entity, the entity then a reference to each of its components.
//   std::vector<std::tuple<RType::Entity, Ts &...>> v =
//       gm.view<Ts...>().getVector();
//   for (auto &[entity, a, b] : gm.view<A, B>().getVector()) { ... }

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

    struct MoveOnly {
        std::unique_ptr<int> ptr;
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

    static_assert(
        std::is_same_v<decltype(std::declval<View<Health> &>().getVector()),
            std::vector<std::tuple<Entity, Health &>>>,
        "getVector must return std::vector<std::tuple<Entity, Ts &...>>");

    static_assert(
        std::is_same_v<
            decltype(std::declval<View<Damage, Health> &>().getVector()),
            std::vector<std::tuple<Entity, Damage &, Health &>>>,
        "getVector must keep the order of the view's types");

    Test(view_vectors, emptyOnNeverAddedType,
        .description = "[view vector] empty on a never added type")
    {
        GameManager gm;

        cr_assert_no_throw(
            (void) gm.view<Health>().getVector(), std::exception);
        cr_assert(gm.view<Health>().getVector().empty());
    }

    Test(view_vectors, emptyWhenOneTypeIsMissing,
        .description = "[view vector] empty when one type is missing")
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        cr_assert_no_throw(
            (void) (gm.view<Health, Damage>().getVector()), std::exception);
        cr_assert((gm.view<Health, Damage>().getVector().empty()));
    }

    Test(view_vectors, emptyWhenNoEntityHasAllTypes,
        .description = "[view vector] empty when no entity has all the types")
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.addComponent<Damage>(gm.createEntity(), 1);
        cr_assert((gm.view<Health, Damage>().getVector().empty()));
    }

    Test(view_vectors, holdsOnlyEntitiesWithAllTypes,
        .description = "[view vector] holds only entities with all the types")
    {
        GameManager gm;
        EntitySet expected;
        EntitySet seen;

        for (int i = 0; i < 30; i++) {
            Entity e = gm.createEntity();
            if (i % 2 == 0)
                gm.addComponent<Health>(e, i);
            if (i % 3 == 0)
                gm.addComponent<Damage>(e, i);
            if (i % 6 == 0)
                expected.insert(e);
        }
        auto v = gm.view<Health, Damage>().getVector();
        cr_assert(v.size() == expected.size());
        for (auto &t : v)
            seen.insert(std::get<0>(t));
        cr_assert(seen.size() == expected.size());
        for (Entity e : expected)
            cr_assert(seen.contains(e));
    }

    Test(view_vectors, componentsMatchTheirEntity,
        .description = "[view vector] components match their entity")
    {
        GameManager gm;
        bool paired = true;

        for (int i = 0; i < 20; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Owned>(e, e, i);
            if (i % 2 == 0)
                gm.addComponent<Health>(e, i);
        }
        for (auto &[e, o, h] : gm.view<Owned, Health>().getVector())
            if (!(o.owner == e) || h.hp != o.value)
                paired = false;
        cr_assert(paired);
    }

    Test(view_vectors, holdsReferencesNotCopies,
        .description = "[view vector] holds references, not copies")
    {
        GameManager gm;
        bool sameAddress = true;

        for (int i = 0; i < 5; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        for (auto &[e, h] : gm.view<Health>().getVector())
            if (&h != &gm.getComponent<Health>(e))
                sameAddress = false;
        cr_assert(sameAddress);
    }

    Test(view_vectors, writesThroughTheVector,
        .description = "[view vector] writes through the vector")
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 5; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            gm.addComponent<Damage>(e, 1);
            entities.push_back(e);
        }
        for (auto &[e, h, d] : gm.view<Health, Damage>().getVector())
            h.hp += d.value * 10;
        for (int i = 0; i < 5; i++)
            cr_assert(gm.getComponent<Health>(entities[i]).hp == i + 10);
    }

    Test(view_vectors, writesThroughStdGet,
        .description = "[view vector] writes through std::get")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        auto v = gm.view<Health>().getVector();
        cr_assert(v.size() == 1);
        std::get<1>(v[0]).hp = 42;
        cr_assert(gm.getComponent<Health>(e).hp == 42);
    }

    Test(view_vectors, sameEntitiesAsEach,
        .description = "[view vector] same entities as each")
    {
        GameManager gm;
        EntitySet fromEach;
        EntitySet fromVector;

        for (int i = 0; i < 50; i++) {
            Entity e = gm.createEntity();
            if (i % 2 == 0)
                gm.addComponent<Position>(e, 0.f, 0.f);
            if (i % 5 != 0)
                gm.addComponent<Velocity>(e, 1.f, 1.f);
        }
        gm.view<Position, Velocity>().each(
            [&fromEach](
                Entity e, Position &, Velocity &) { fromEach.insert(e); });
        for (auto &[e, p, v] : gm.view<Position, Velocity>().getVector())
            fromVector.insert(e);
        cr_assert(fromVector.size() == fromEach.size());
        for (Entity e : fromEach)
            cr_assert(fromVector.contains(e));
    }

    Test(view_vectors, typeOrderDoesNotChangeTheEntities,
        .description =
            "[view vector] the type order does not change the entities")
    {
        GameManager gm;
        EntitySet first;
        EntitySet second;

        for (int i = 0; i < 40; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            if (i % 4 == 0)
                gm.addComponent<Damage>(e, i);
        }
        for (auto &[e, h, d] : gm.view<Health, Damage>().getVector())
            first.insert(e);
        for (auto &[e, d, h] : gm.view<Damage, Health>().getVector())
            second.insert(e);
        cr_assert(first.size() == 10);
        cr_assert(second.size() == first.size());
        for (Entity e : first)
            cr_assert(second.contains(e));
    }

    Test(view_vectors, worksWithEmptyTags,
        .description = "[view vector] works with empty tags")
    {
        GameManager gm;
        int count = 0;

        for (int i = 0; i < 6; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            if (i % 2 == 0)
                gm.addComponent<Enemy>(e);
        }
        for (auto &[e, tag, h] : gm.view<Enemy, Health>().getVector())
            count += h.hp % 2 == 0 ? 1 : 0;
        cr_assert(count == 3);
    }

    Test(view_vectors, worksOnAViewBuiltFromAComponentManager,
        .description =
            "[view vector] works on a view built from a ComponentManager")
    {
        ComponentManager cm;
        EntityManager em;

        for (int i = 0; i < 4; i++)
            cm.addComponent<Health>(em.createEntity(), i);
        View<Health> view(cm);
        cr_assert(view.getVector().size() == 4);
    }

    Test(view_vectors, eachCallGivesAFreshVector,
        .description = "[view vector] each call gives a fresh vector")
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto view = gm.view<Health>();
        auto first = view.getVector();
        gm.addComponent<Health>(gm.createEntity(), 2);
        auto second = view.getVector();
        cr_assert(first.size() == 1);
        cr_assert(second.size() == 2);
    }

    Test(
        view_vectors, isASnapshot, .description = "[view vector] is a snapshot")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        auto v = gm.view<Health>().getVector();
        gm.addComponent<Health>(gm.createEntity(), 2);
        cr_assert(v.size() == 1);
        cr_assert(std::get<0>(v[0]) == e);
    }

    Test(view_vectors, referencesSurviveOtherRemovals,
        .description = "[view vector] references survive other removals")
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
        }
        auto v = gm.view<Health>().getVector();
        for (int i = 0; i < 10; i += 2)
            gm.destroyEntity(entities[i]);
        for (int i = 0; i < 30; i++)
            gm.addComponent<Health>(gm.createEntity(), 100 + i);
        bool intact = true;
        for (auto &[e, h] : v) {
            if (!gm.hasEntity(e))
                continue;
            if (&h != &gm.getComponent<Health>(e) ||
                h.hp != static_cast<int>(e.index()))
                intact = false;
        }
        cr_assert(intact);
    }

    Test(view_vectors, doesNotCopyTheComponents,
        .description = "[view vector] does not copy the components")
    {
        Tracked::alive = 0;
        GameManager gm;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Tracked>(gm.createEntity(), i);
        auto v = gm.view<Tracked>().getVector();
        cr_assert(v.size() == 3);
        cr_assert(Tracked::alive == 3);
    }

    Test(view_vectors, moveOnlyComponents,
        .description = "[view vector] move-only components")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<MoveOnly>(e, std::make_unique<int>(7));
        auto v = gm.view<MoveOnly>().getVector();
        cr_assert(v.size() == 1);
        cr_assert(*std::get<1>(v[0]).ptr == 7);
    }

    Test(view_vectors, destroyFromTheVectorDuringUpdate,
        .description = "[view vector] destroy from the vector during update")
    {
        GameManager gm;
        std::vector<Entity> bullets;
        bool aliveDuringFrame = true;

        for (int i = 0; i < 5; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Bullet>(e);
            bullets.push_back(e);
        }
        gm.createSystem([&aliveDuringFrame](GameManager &g) {
            for (auto &[e, b] : g.view<Bullet>().getVector()) {
                g.destroyEntity(e);
                aliveDuringFrame = aliveDuringFrame && g.hasEntity(e);
            }
        });
        gm.update();
        cr_assert(aliveDuringFrame);
        for (Entity e : bullets)
            cr_assert(!gm.hasEntity(e));
    }

    Test(view_vectors, destroyFromTheVectorOutsideUpdate,
        .description = "[view vector] destroy from the vector outside update")
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 6; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
        }
        for (auto &[e, h] : gm.view<Health>().getVector())
            if (h.hp % 2 == 0)
                gm.destroyEntity(e);
        for (int i = 0; i < 6; i++)
            cr_assert(gm.hasEntity(entities[i]) == (i % 2 == 1));
        cr_assert(gm.view<Health>().getVector().size() == 3);
    }

    Test(view_vectors, collisionsWithVectors,
        .description = "[view vector] bullets against enemies with vectors")
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
            auto targets = g.view<Enemy, Position, Health>().getVector();
            for (auto &[bullet, b, bp, d] :
                g.view<Bullet, Position, Damage>().getVector()) {
                for (auto &[enemy, en, ep, h] : targets) {
                    if (ep.x != bp.x)
                        continue;
                    h.hp -= d.value;
                    g.destroyEntity(bullet);
                }
            }
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

    Test(view_vectors, manyEntities,
        .description = "[view vector] many entities")
    {
        GameManager gm;
        int sum = 0;

        for (int i = 0; i < 1000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, 1);
            if (i % 2 == 0)
                gm.addComponent<Damage>(e, 2);
        }
        auto v = gm.view<Health, Damage>().getVector();
        for (auto &[e, h, d] : v)
            sum += h.hp + d.value;
        cr_assert(v.size() == 500);
        cr_assert(sum == 1500);
    }

} // namespace
