/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EdgeCases
*/

#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

namespace {

    using RType::Entity;
    using RType::GameManager;
    using namespace Poc;

    struct NeverAdded {
        int unused;
    };

    void addReturnsStoredComponent()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Position &pos = gm.addComponent<Position>(e, 1.f, 2.f);

        POC_CHECK(pos.x == 1.f && pos.y == 2.f);
        pos.x = 10.f;
        POC_CHECK(gm.getComponent<Position>(e).x == 10.f);
        POC_CHECK(&gm.getComponent<Position>(e) == &pos);
    }

    void addWithoutArgumentsValueInitializes()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Position &pos = gm.addComponent<Position>(e);

        POC_CHECK(pos.x == 0.f && pos.y == 0.f);
    }

    void firstEntityIdZeroWorks()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        POC_CHECK(e == 0);
        gm.addComponent<Health>(e, 3);
        POC_CHECK(gm.hasComponent<Health>(e));
        gm.removeComponent<Health>(e);
        POC_CHECK(!gm.hasComponent<Health>(e));
    }

    void eachEntityHasItsOwnComponent()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();
        Entity c = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        gm.addComponent<Health>(b, 2);
        gm.addComponent<Health>(c, 3);
        gm.getComponent<Health>(b).hp = 20;
        POC_CHECK(gm.getComponent<Health>(a).hp == 1);
        POC_CHECK(gm.getComponent<Health>(b).hp == 20);
        POC_CHECK(gm.getComponent<Health>(c).hp == 3);
    }

    void sameLayoutTypesDoNotCollide()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        gm.addComponent<Velocity>(e, 5.f, 5.f);
        POC_CHECK(gm.getComponent<Position>(e).x == 1.f);
        POC_CHECK(gm.getComponent<Velocity>(e).x == 5.f);
        POC_CHECK(gm.getArray<Position>().nbComponent() == 1);
        POC_CHECK(gm.getArray<Velocity>().nbComponent() == 1);
    }

    void addTwiceThrows()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 5);
        POC_CHECK_THROWS(
            gm.addComponent<Health>(e, 99), RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Health>(e).hp == 5);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void getMissingThrows()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        POC_CHECK_THROWS(gm.getComponent<Health>(b), RType::NoSuchComponent);
        POC_CHECK_THROWS(
            gm.getComponent<NeverAdded>(a), RType::NoSuchComponent);
    }

    void hasFalseForUnknownType()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool has = true;

        POC_CHECK_NOTHROW(has = gm.hasComponent<NeverAdded>(e));
        POC_CHECK(!has);
    }

    void hasThroughConstGameManager()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();
        const GameManager &cgm = gm;

        gm.addComponent<Health>(a, 1);
        POC_CHECK(cgm.hasComponent<Health>(a));
        POC_CHECK(!cgm.hasComponent<Health>(b));
    }

    void removeOnlyTargetsOneComponent()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Velocity>(a, 2.f, 2.f);
        gm.addComponent<Position>(b, 3.f, 3.f);
        gm.removeComponent<Position>(a);
        POC_CHECK(!gm.hasComponent<Position>(a));
        POC_CHECK(gm.hasComponent<Velocity>(a));
        POC_CHECK(gm.getComponent<Velocity>(a).x == 2.f);
        POC_CHECK(gm.getComponent<Position>(b).x == 3.f);
        POC_CHECK(gm.getArray<Position>().nbComponent() == 1);
    }

    void removeMissingThrows()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        POC_CHECK_THROWS(gm.removeComponent<Health>(b), RType::NoSuchComponent);
        POC_CHECK_THROWS(
            gm.removeComponent<NeverAdded>(a), RType::NoSuchComponent);
        POC_CHECK(gm.getComponent<Health>(a).hp == 1);
    }

    void reAddAfterRemove()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.removeComponent<Health>(e);
        gm.addComponent<Health>(e, 2);
        POC_CHECK(gm.getComponent<Health>(e).hp == 2);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void destroyRemovesEveryComponent()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Velocity>(a, 1.f, 1.f);
        gm.addComponent<Health>(a, 1);
        gm.addComponent<Health>(b, 2);
        gm.destroyEntity(a);
        POC_CHECK(!gm.hasComponent<Position>(a));
        POC_CHECK(!gm.hasComponent<Velocity>(a));
        POC_CHECK(!gm.hasComponent<Health>(a));
        POC_CHECK(gm.getArray<Position>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Velocity>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
        POC_CHECK(gm.getComponent<Health>(b).hp == 2);
    }

    void destroyWithSomeComponentTypesOnly()
    {
        GameManager gm;
        Entity b = gm.createEntity();
        Entity a = b;

        gm.addComponent<Velocity>(b, 7.f, 7.f);
        for (int i = 0; i < 50; i++)
            a = gm.createEntity();
        gm.addComponent<Position>(a, 1.f, 1.f);
        POC_CHECK_NOTHROW(gm.destroyEntity(a));
        POC_CHECK(!gm.hasComponent<Position>(a));
        POC_CHECK(gm.getComponent<Velocity>(b).x == 7.f);
    }

    void destroyWithoutAnyComponent()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        POC_CHECK_NOTHROW(gm.destroyEntity(b));
        POC_CHECK(gm.getComponent<Health>(a).hp == 1);
    }

    void destroyTwiceDoesNotCrash()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(b, 5);
        gm.destroyEntity(a);
        try {
            gm.destroyEntity(a);
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(gm.getComponent<Health>(b).hp == 5);
    }

    void entityIdsAreUnique()
    {
        GameManager gm;
        std::vector<Entity> created;
        std::set<Entity> alive;

        for (int i = 0; i < 100; i++)
            created.push_back(gm.createEntity());
        for (std::size_t i = 0; i < created.size(); i++) {
            if (i % 2 == 0)
                gm.destroyEntity(created[i]);
            else
                alive.insert(created[i]);
        }
        for (int i = 0; i < 100; i++) {
            Entity e = gm.createEntity();
            POC_CHECK(!alive.contains(e));
            alive.insert(e);
        }
        POC_CHECK(alive.size() == 150);
    }

    void newEntityDoesNotInheritComponents()
    {
        GameManager gm;
        Entity a = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Health>(a, 1);
        gm.destroyEntity(a);
        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            POC_CHECK(!gm.hasComponent<Position>(e));
            POC_CHECK(!gm.hasComponent<Health>(e));
        }
    }

    void referencesSurviveInsertions()
    {
        GameManager gm;
        Entity first = gm.createEntity();
        Position *ptr = &gm.addComponent<Position>(first, 1.f, 2.f);
        std::vector<Entity> others;

        for (int i = 0; i < 10000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Position>(e, static_cast<float>(i), 0.f);
            others.push_back(e);
        }
        POC_CHECK(&gm.getComponent<Position>(first) == ptr);
        POC_CHECK(ptr->x == 1.f && ptr->y == 2.f);
        for (std::size_t i = 0; i < others.size(); i++)
            POC_CHECK(gm.getComponent<Position>(others[i]).x ==
                static_cast<float>(i));
    }

    void referencesSurviveRemovals()
    {
        GameManager gm;
        std::vector<Entity> entities;
        std::vector<Position *> ptrs;

        for (int i = 0; i < 5000; i++) {
            Entity e = gm.createEntity();
            entities.push_back(e);
            ptrs.push_back(
                &gm.addComponent<Position>(e, static_cast<float>(i), 0.f));
        }
        for (std::size_t i = 0; i < entities.size(); i += 2)
            gm.destroyEntity(entities[i]);
        for (std::size_t i = 1; i < entities.size(); i += 2) {
            POC_CHECK(&gm.getComponent<Position>(entities[i]) == ptrs[i]);
            POC_CHECK(ptrs[i]->x == static_cast<float>(i));
        }
        POC_CHECK(gm.getArray<Position>().nbComponent() == 2500);
    }

    void largeIdsGrowSparse()
    {
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 5000; i++)
            entities.push_back(gm.createEntity());
        gm.addComponent<Health>(entities.back(), 42);
        for (std::size_t i = 0; i + 1 < entities.size(); i++)
            POC_CHECK(!gm.hasComponent<Health>(entities[i]));
        POC_CHECK(gm.getComponent<Health>(entities.back()).hp == 42);
        gm.addComponent<Health>(entities.front(), 1);
        POC_CHECK(gm.getComponent<Health>(entities.front()).hp == 1);
        POC_CHECK(gm.getComponent<Health>(entities.back()).hp == 42);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 2);
    }

    void denseIterationSeesEveryComponent()
    {
        GameManager gm;
        std::vector<Entity> entities;
        int expected = 0;

        for (int i = 1; i <= 100; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            entities.push_back(e);
            expected += i;
        }
        for (std::size_t i = 0; i < entities.size(); i += 3) {
            expected -= gm.getComponent<Health>(entities[i]).hp;
            gm.removeComponent<Health>(entities[i]);
        }
        int sum = 0;
        std::size_t count = 0;
        for (const Health &health : gm.getArray<Health>().getDense()) {
            sum += health.hp;
            count++;
        }
        POC_CHECK(sum == expected);
        POC_CHECK(count == gm.getArray<Health>().nbComponent());
        POC_CHECK(count == 66);
    }

    void getArrayOnUnknownType()
    {
        GameManager gm;

        try {
            auto &array = gm.getArray<NeverAdded>();
            POC_CHECK(array.nbComponent() == 0);
        } catch (const RType::EngineException &) {
        } catch (const std::exception &e) {
            throw Failure(__FILE__,
                __LINE__,
                "getArray<NeverAdded>() leaked a non engine exception: " +
                    std::string(e.what()));
        }
    }

    void destructorsRun()
    {
        Tracked::alive = 0;
        {
            GameManager gm;
            Entity a = gm.createEntity();
            Entity b = gm.createEntity();
            Entity c = gm.createEntity();

            gm.addComponent<Tracked>(a, 1);
            gm.addComponent<Tracked>(b, 2);
            gm.addComponent<Tracked>(c, 3);
            POC_CHECK(Tracked::alive == 3);
            gm.removeComponent<Tracked>(a);
            POC_CHECK(Tracked::alive == 2);
            gm.destroyEntity(b);
            POC_CHECK(Tracked::alive == 1);
            POC_CHECK_THROWS(gm.addComponent<Tracked>(c, 9),
                RType::AlreadyExistingComponent);
            POC_CHECK(Tracked::alive == 1);
        }
        POC_CHECK(Tracked::alive == 0);
    }

    void moveOnlyComponent()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<MoveOnly>(e, std::make_unique<int>(42));
        POC_CHECK(gm.getComponent<MoveOnly>(e).ptr != nullptr);
        POC_CHECK(*gm.getComponent<MoveOnly>(e).ptr == 42);
        gm.destroyEntity(e);
    }

    void heapComponentsSurvive()
    {
        GameManager gm;
        std::vector<Entity> entities;
        auto nameOf = [](std::size_t i) {
            return "an entity name long enough to avoid SSO #" +
                std::to_string(i);
        };

        for (std::size_t i = 0; i < 2000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Name>(e, nameOf(i));
            entities.push_back(e);
        }
        for (std::size_t i = 0; i < entities.size(); i += 3)
            gm.removeComponent<Name>(entities[i]);
        for (std::size_t i = 0; i < entities.size(); i++) {
            if (i % 3 == 0)
                POC_CHECK(!gm.hasComponent<Name>(entities[i]));
            else
                POC_CHECK(
                    gm.getComponent<Name>(entities[i]).value == nameOf(i));
        }
    }

    void emptyTagComponent()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Player>(a);
        POC_CHECK(gm.hasComponent<Player>(a));
        POC_CHECK(!gm.hasComponent<Player>(b));
        POC_CHECK(gm.getArray<Player>().nbComponent() == 1);
        gm.removeComponent<Player>(a);
        POC_CHECK(!gm.hasComponent<Player>(a));
    }

    void addOnDestroyedEntityIsRejected()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.destroyEntity(e);
        POC_CHECK_THROWS(gm.addComponent<Health>(e, 1), RType::EngineException);
    }

    void randomOperationsMatchModel()
    {
        GameManager gm;
        std::mt19937 rng(42);
        std::vector<Entity> alive;
        std::vector<Entity> dead;
        std::map<Entity, int> health;
        std::map<Entity, float> position;

        auto verify = [&]() {
            int sum = 0;
            for (Entity e : alive) {
                POC_CHECK(gm.hasComponent<Health>(e) == health.contains(e));
                POC_CHECK(gm.hasComponent<Position>(e) == position.contains(e));
                if (health.contains(e))
                    POC_CHECK(gm.getComponent<Health>(e).hp == health[e]);
                if (position.contains(e))
                    POC_CHECK(gm.getComponent<Position>(e).x == position[e]);
            }
            for (Entity e : dead) {
                POC_CHECK(!gm.hasComponent<Health>(e));
                POC_CHECK(!gm.hasComponent<Position>(e));
            }
            for (const Health &h : gm.getArray<Health>().getDense())
                sum += h.hp;
            for (const auto &[_, hp] : health)
                sum -= hp;
            POC_CHECK(sum == 0);
            POC_CHECK(gm.getArray<Health>().nbComponent() == health.size());
            POC_CHECK(gm.getArray<Position>().nbComponent() == position.size());
        };

        for (int i = 0; i < 20000; i++) {
            unsigned op = rng() % 100;
            if (alive.empty() || op < 15) {
                alive.push_back(gm.createEntity());
                continue;
            }
            std::size_t idx = rng() % alive.size();
            Entity e = alive[idx];
            int value = static_cast<int>(rng() % 1000);
            if (op < 25) {
                gm.destroyEntity(e);
                alive.erase(alive.begin() + static_cast<std::ptrdiff_t>(idx));
                dead.push_back(e);
                health.erase(e);
                position.erase(e);
            } else if (op < 45) {
                if (health.contains(e)) {
                    POC_CHECK_THROWS(gm.addComponent<Health>(e, value),
                        RType::AlreadyExistingComponent);
                } else {
                    gm.addComponent<Health>(e, value);
                    health[e] = value;
                }
            } else if (op < 60) {
                if (health.contains(e)) {
                    gm.removeComponent<Health>(e);
                    health.erase(e);
                } else {
                    POC_CHECK_THROWS(
                        gm.removeComponent<Health>(e), RType::NoSuchComponent);
                }
            } else if (op < 75) {
                if (position.contains(e)) {
                    gm.removeComponent<Position>(e);
                    position.erase(e);
                } else {
                    gm.addComponent<Position>(
                        e, static_cast<float>(value), 0.f);
                    position[e] = static_cast<float>(value);
                }
            } else {
                if (health.contains(e)) {
                    gm.getComponent<Health>(e).hp += 1;
                    health[e] += 1;
                } else {
                    POC_CHECK_THROWS(
                        gm.getComponent<Health>(e), RType::NoSuchComponent);
                }
            }
            if (i % 1000 == 999)
                verify();
        }
        verify();
    }

} // namespace

namespace Poc {

    TestList edgeCaseTests()
    {
        return {
            {"addComponent returns the stored component",
                addReturnsStoredComponent},
            {"addComponent without arguments value-initializes",
                addWithoutArgumentsValueInitializes},
            {"entity 0 behaves like any other entity", firstEntityIdZeroWorks},
            {"each entity has its own component", eachEntityHasItsOwnComponent},
            {"types with the same layout do not collide",
                sameLayoutTypesDoNotCollide},
            {"adding the same component twice throws", addTwiceThrows},
            {"getComponent on a missing component throws", getMissingThrows},
            {"hasComponent is false for a type never added",
                hasFalseForUnknownType},
            {"hasComponent works through a const GameManager",
                hasThroughConstGameManager},
            {"removeComponent only removes that component",
                removeOnlyTargetsOneComponent},
            {"removeComponent on a missing component throws",
                removeMissingThrows},
            {"a component can be re-added after removal", reAddAfterRemove},
            {"destroyEntity removes components of every type",
                destroyRemovesEveryComponent},
            {"destroyEntity with only some component types",
                destroyWithSomeComponentTypesOnly},
            {"destroyEntity on an entity without components",
                destroyWithoutAnyComponent},
            {"destroying an entity twice does not crash",
                destroyTwiceDoesNotCrash},
            {"entity ids stay unique after destructions", entityIdsAreUnique},
            {"a new entity never inherits old components",
                newEntityDoesNotInheritComponents},
            {"references survive 10000 insertions",
                referencesSurviveInsertions},
            {"references survive removals", referencesSurviveRemovals},
            {"large entity ids grow the sparse array", largeIdsGrowSparse},
            {"dense iteration sees every live component",
                denseIterationSeesEveryComponent},
            {"[design] getArray on a type never added", getArrayOnUnknownType},
            {"destructors run on remove, destroy and shutdown", destructorsRun},
            {"move-only components", moveOnlyComponent},
            {"heap-owning components survive insertions and removals",
                heapComponentsSurvive},
            {"empty tag components", emptyTagComponent},
            {"[design] adding a component to a destroyed entity is rejected",
                addOnDestroyedEntityIsRejected},
            {"20000 random operations match a reference model",
                randomOperationsMatchModel},
        };
    }

} // namespace Poc
