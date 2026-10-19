/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ViewVectors
*/

#include <tuple>
#include <type_traits>
#include <vector>

#include "ComponentManager.hpp"
#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"
#include "View.hpp"

// Expected API: a view can be turned into a vector holding, for each
// matching entity, the entity then a reference to each of its components.
//   std::vector<std::tuple<RType::Entity, Ts &...>> v =
//       gm.view<Ts...>().getVector();
//   for (auto &[entity, a, b] : gm.view<A, B>().getVector()) { ... }

namespace {

    using RType::ComponentManager;
    using RType::Entity;
    using RType::EntityManager;
    using RType::GameManager;
    using RType::View;
    using namespace Poc;

    static_assert(std::is_same_v<decltype(std::declval<View<Health> &>()
                                             .getVector()),
                      std::vector<std::tuple<Entity, Health &>>>,
        "getVector must return std::vector<std::tuple<Entity, Ts &...>>");

    static_assert(
        std::is_same_v<decltype(std::declval<View<Damage, Health> &>()
                                    .getVector()),
            std::vector<std::tuple<Entity, Damage &, Health &>>>,
        "getVector must keep the order of the view's types");

    void emptyOnNeverAddedType()
    {
        GameManager gm;

        POC_CHECK_NOTHROW((void) gm.view<Health>().getVector());
        POC_CHECK(gm.view<Health>().getVector().empty());
    }

    void emptyWhenOneTypeIsMissing()
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        POC_CHECK_NOTHROW((void) (gm.view<Health, Damage>().getVector()));
        POC_CHECK((gm.view<Health, Damage>().getVector().empty()));
    }

    void emptyWhenNoEntityHasAllTypes()
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        gm.addComponent<Damage>(gm.createEntity(), 1);
        POC_CHECK((gm.view<Health, Damage>().getVector().empty()));
    }

    void holdsOnlyEntitiesWithAllTypes()
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
        POC_CHECK(v.size() == expected.size());
        for (auto &t : v)
            seen.insert(std::get<0>(t));
        POC_CHECK(seen.size() == expected.size());
        for (Entity e : expected)
            POC_CHECK(seen.contains(e));
    }

    void componentsMatchTheirEntity()
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
        POC_CHECK(paired);
    }

    void holdsReferencesNotCopies()
    {
        GameManager gm;
        bool sameAddress = true;

        for (int i = 0; i < 5; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        for (auto &[e, h] : gm.view<Health>().getVector())
            if (&h != &gm.getComponent<Health>(e))
                sameAddress = false;
        POC_CHECK(sameAddress);
    }

    void writesThroughTheVector()
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
            POC_CHECK(gm.getComponent<Health>(entities[i]).hp == i + 10);
    }

    void writesThroughStdGet()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        auto v = gm.view<Health>().getVector();
        POC_CHECK(v.size() == 1);
        std::get<1>(v[0]).hp = 42;
        POC_CHECK(gm.getComponent<Health>(e).hp == 42);
    }

    void sameEntitiesAsEach()
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
            [&fromEach](Entity e, Position &, Velocity &) {
                fromEach.insert(e);
            });
        for (auto &[e, p, v] : gm.view<Position, Velocity>().getVector())
            fromVector.insert(e);
        POC_CHECK(fromVector.size() == fromEach.size());
        for (Entity e : fromEach)
            POC_CHECK(fromVector.contains(e));
    }

    void typeOrderDoesNotChangeTheEntities()
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
        POC_CHECK(first.size() == 10);
        POC_CHECK(second.size() == first.size());
        for (Entity e : first)
            POC_CHECK(second.contains(e));
    }

    void worksWithEmptyTags()
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
        POC_CHECK(count == 3);
    }

    void worksOnAViewBuiltFromAComponentManager()
    {
        ComponentManager cm;
        EntityManager em;

        for (int i = 0; i < 4; i++)
            cm.addComponent<Health>(em.createEntity(), i);
        View<Health> view(cm);
        POC_CHECK(view.getVector().size() == 4);
    }

    void eachCallGivesAFreshVector()
    {
        GameManager gm;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto view = gm.view<Health>();
        auto first = view.getVector();
        gm.addComponent<Health>(gm.createEntity(), 2);
        auto second = view.getVector();
        POC_CHECK(first.size() == 1);
        POC_CHECK(second.size() == 2);
    }

    void isASnapshot()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        auto v = gm.view<Health>().getVector();
        gm.addComponent<Health>(gm.createEntity(), 2);
        POC_CHECK(v.size() == 1);
        POC_CHECK(std::get<0>(v[0]) == e);
    }

    void referencesSurviveOtherRemovals()
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
        POC_CHECK(intact);
    }

    void doesNotCopyTheComponents()
    {
        Tracked::alive = 0;
        GameManager gm;

        for (int i = 0; i < 3; i++)
            gm.addComponent<Tracked>(gm.createEntity(), i);
        auto v = gm.view<Tracked>().getVector();
        POC_CHECK(v.size() == 3);
        POC_CHECK(Tracked::alive == 3);
    }

    void moveOnlyComponents()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<MoveOnly>(e, std::make_unique<int>(7));
        auto v = gm.view<MoveOnly>().getVector();
        POC_CHECK(v.size() == 1);
        POC_CHECK(*std::get<1>(v[0]).ptr == 7);
    }

    void destroyFromTheVectorDuringUpdate()
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
        POC_CHECK(aliveDuringFrame);
        for (Entity e : bullets)
            POC_CHECK(!gm.hasEntity(e));
    }

    void destroyFromTheVectorOutsideUpdate()
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
            POC_CHECK(gm.hasEntity(entities[i]) == (i % 2 == 1));
        POC_CHECK(gm.view<Health>().getVector().size() == 3);
    }

    void collisionsWithVectors()
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
        POC_CHECK(!gm.hasEntity(bullets[0]));
        POC_CHECK(!gm.hasEntity(bullets[1]));
        POC_CHECK(gm.hasEntity(bullets[2]));
        POC_CHECK(gm.getComponent<Health>(enemies[0]).hp == 2);
        POC_CHECK(gm.getComponent<Health>(enemies[1]).hp == 3);
        POC_CHECK(gm.getComponent<Health>(enemies[2]).hp == 2);
        POC_CHECK(gm.getComponent<Health>(enemies[3]).hp == 3);
    }

    void manyEntities()
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
        POC_CHECK(v.size() == 500);
        POC_CHECK(sum == 1500);
    }

} // namespace

namespace Poc {

    TestList viewVectorTests()
    {
        return {
            {"[view vector] empty on a never added type",
                emptyOnNeverAddedType},
            {"[view vector] empty when one type is missing",
                emptyWhenOneTypeIsMissing},
            {"[view vector] empty when no entity has all the types",
                emptyWhenNoEntityHasAllTypes},
            {"[view vector] holds only entities with all the types",
                holdsOnlyEntitiesWithAllTypes},
            {"[view vector] components match their entity",
                componentsMatchTheirEntity},
            {"[view vector] holds references, not copies",
                holdsReferencesNotCopies},
            {"[view vector] writes through the vector",
                writesThroughTheVector},
            {"[view vector] writes through std::get", writesThroughStdGet},
            {"[view vector] same entities as each", sameEntitiesAsEach},
            {"[view vector] the type order does not change the entities",
                typeOrderDoesNotChangeTheEntities},
            {"[view vector] works with empty tags", worksWithEmptyTags},
            {"[view vector] works on a view built from a ComponentManager",
                worksOnAViewBuiltFromAComponentManager},
            {"[view vector] each call gives a fresh vector",
                eachCallGivesAFreshVector},
            {"[view vector] is a snapshot", isASnapshot},
            {"[view vector] references survive other removals",
                referencesSurviveOtherRemovals},
            {"[view vector] does not copy the components",
                doesNotCopyTheComponents},
            {"[view vector] move-only components", moveOnlyComponents},
            {"[view vector] destroy from the vector during update",
                destroyFromTheVectorDuringUpdate},
            {"[view vector] destroy from the vector outside update",
                destroyFromTheVectorOutsideUpdate},
            {"[view vector] bullets against enemies with vectors",
                collisionsWithVectors},
            {"[view vector] many entities", manyEntities},
        };
    }

} // namespace Poc
