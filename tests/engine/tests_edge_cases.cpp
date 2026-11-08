/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_edge_cases
*/

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <criterion/criterion.h>

#include "Entity.hpp"
#include "GameManager.hpp"

namespace {

    struct Player {};

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

    struct NeverAdded {
        int unused;
    };

    Test(edge_cases, addIsImmediateOutsideUpdate,
        .description = "addComponent is immediate outside update")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 2.f);
        auto &pos = gm.getComponent<Position>(e);
        cr_assert(pos.x == 1.f && pos.y == 2.f);
        pos.x = 10.f;
        cr_assert(gm.getComponent<Position>(e).x == 10.f);
        cr_assert(&gm.getComponent<Position>(e) == &pos);
    }

    Test(edge_cases, addWithoutArgumentsValueInitializes,
        .description = "addComponent without arguments value-initializes")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e);
        const Position &pos = gm.getComponent<Position>(e);
        cr_assert(pos.x == 0.f && pos.y == 0.f);
    }

    Test(edge_cases, failedCommandDoesNotReplay,
        .description = "a failed command does not replay on later operations")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        cr_assert_throw(
            gm.addComponent<Health>(e, 2), RType::AlreadyExistingComponent);
        cr_assert_no_throw(
            gm.addComponent<Position>(e, 3.f, 3.f), std::exception);
        cr_assert_no_throw(gm.removeComponent<Health>(e), std::exception);
        cr_assert(!gm.hasComponent<Health>(e));
        cr_assert(gm.getComponent<Position>(e).x == 3.f);
    }

    Test(edge_cases, addWithTemporaryArguments,
        .description = "addComponent with temporary arguments")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Name>(e, std::string(64, 'a') + "b");
        cr_assert(gm.getComponent<Name>(e).value == std::string(64, 'a') + "b");
    }

    Test(edge_cases, firstEntityIdZeroWorks,
        .description = "entity 0 behaves like any other entity")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        cr_assert(indexOf(e) == 0);
        gm.addComponent<Health>(e, 3);
        cr_assert(gm.hasComponent<Health>(e));
        gm.removeComponent<Health>(e);
        cr_assert(!gm.hasComponent<Health>(e));
    }

    Test(edge_cases, eachEntityHasItsOwnComponent,
        .description = "each entity has its own component")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();
        Entity c = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        gm.addComponent<Health>(b, 2);
        gm.addComponent<Health>(c, 3);
        gm.getComponent<Health>(b).hp = 20;
        cr_assert(gm.getComponent<Health>(a).hp == 1);
        cr_assert(gm.getComponent<Health>(b).hp == 20);
        cr_assert(gm.getComponent<Health>(c).hp == 3);
    }

    Test(edge_cases, sameLayoutTypesDoNotCollide,
        .description = "types with the same layout do not collide")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        gm.addComponent<Velocity>(e, 5.f, 5.f);
        cr_assert(gm.getComponent<Position>(e).x == 1.f);
        cr_assert(gm.getComponent<Velocity>(e).x == 5.f);
        cr_assert(gm.getArray<Position>().nbComponent() == 1);
        cr_assert(gm.getArray<Velocity>().nbComponent() == 1);
    }

    Test(edge_cases, addTwiceThrows,
        .description = "adding the same component twice throws")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 5);
        cr_assert_throw(
            gm.addComponent<Health>(e, 99), RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Health>(e).hp == 5);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(edge_cases, getMissingThrows,
        .description = "getComponent on a missing component throws")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        cr_assert_throw(static_cast<void>(gm.getComponent<Health>(b)),
            RType::NoSuchComponent);
        cr_assert_throw(static_cast<void>(gm.getComponent<NeverAdded>(a)),
            RType::NoSuchComponent);
    }

    Test(edge_cases, hasFalseForUnknownType,
        .description = "hasComponent is false for a type never added")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool has = true;

        cr_assert_no_throw(
            has = gm.hasComponent<NeverAdded>(e), std::exception);
        cr_assert(!has);
    }

    Test(edge_cases, hasThroughConstGameManager,
        .description = "hasComponent works through a const GameManager")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();
        const GameManager &cgm = gm;

        gm.addComponent<Health>(a, 1);
        cr_assert(cgm.hasComponent<Health>(a));
        cr_assert(!cgm.hasComponent<Health>(b));
    }

    Test(edge_cases, removeOnlyTargetsOneComponent,
        .description = "removeComponent only removes that component")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Velocity>(a, 2.f, 2.f);
        gm.addComponent<Position>(b, 3.f, 3.f);
        gm.removeComponent<Position>(a);
        cr_assert(!gm.hasComponent<Position>(a));
        cr_assert(gm.hasComponent<Velocity>(a));
        cr_assert(gm.getComponent<Velocity>(a).x == 2.f);
        cr_assert(gm.getComponent<Position>(b).x == 3.f);
        cr_assert(gm.getArray<Position>().nbComponent() == 1);
    }

    Test(edge_cases, removeMissingThrows,
        .description = "removeComponent on a missing component throws")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        cr_assert_throw(gm.removeComponent<Health>(b), RType::NoSuchComponent);
        cr_assert_throw(
            gm.removeComponent<NeverAdded>(a), RType::NoSuchComponent);
        cr_assert(gm.getComponent<Health>(a).hp == 1);
    }

    Test(edge_cases, reAddAfterRemove,
        .description = "a component can be re-added after removal")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.removeComponent<Health>(e);
        gm.addComponent<Health>(e, 2);
        cr_assert(gm.getComponent<Health>(e).hp == 2);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(edge_cases, destroyRemovesEveryComponent,
        .description = "destroyEntity removes components of every type")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Velocity>(a, 1.f, 1.f);
        gm.addComponent<Health>(a, 1);
        gm.addComponent<Health>(b, 2);
        gm.destroyEntity(a);
        cr_assert(!gm.hasComponent<Position>(a));
        cr_assert(!gm.hasComponent<Velocity>(a));
        cr_assert(!gm.hasComponent<Health>(a));
        cr_assert(gm.getArray<Position>().nbComponent() == 0);
        cr_assert(gm.getArray<Velocity>().nbComponent() == 0);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
        cr_assert(gm.getComponent<Health>(b).hp == 2);
    }

    Test(edge_cases, destroyWithSomeComponentTypesOnly,
        .description = "destroyEntity with only some component types")
    {
        GameManager gm;
        Entity b = gm.createEntity();
        Entity a = b;

        gm.addComponent<Velocity>(b, 7.f, 7.f);
        for (int i = 0; i < 50; i++)
            a = gm.createEntity();
        gm.addComponent<Position>(a, 1.f, 1.f);
        cr_assert_no_throw(gm.destroyEntity(a), std::exception);
        cr_assert(!gm.hasComponent<Position>(a));
        cr_assert(gm.getComponent<Velocity>(b).x == 7.f);
    }

    Test(edge_cases, destroyWithoutAnyComponent,
        .description = "destroyEntity on an entity without components")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        cr_assert_no_throw(gm.destroyEntity(b), std::exception);
        cr_assert(gm.getComponent<Health>(a).hp == 1);
    }

    Test(edge_cases, destroyTwiceDoesNotCrash,
        .description = "destroying an entity twice does not crash")
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
        cr_assert(!gm.hasEntity(a));
        cr_assert(gm.hasEntity(b));
        cr_assert(gm.getComponent<Health>(b).hp == 5);
    }

    Test(edge_cases, entityIdsAreUnique,
        .description = "entity ids stay unique after destructions")
    {
        GameManager gm;
        std::vector<Entity> created;
        EntitySet alive;

        created.reserve(100);
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
            cr_assert(!alive.contains(e));
            alive.insert(e);
        }
        cr_assert(alive.size() == 150);
    }

    Test(edge_cases, newEntityDoesNotInheritComponents,
        .description = "a new entity never inherits old components")
    {
        GameManager gm;
        Entity a = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Health>(a, 1);
        gm.destroyEntity(a);
        for (int i = 0; i < 10; i++) {
            Entity e = gm.createEntity();
            cr_assert(!gm.hasComponent<Position>(e));
            cr_assert(!gm.hasComponent<Health>(e));
        }
    }

    Test(edge_cases, referencesSurviveInsertions,
        .description = "references survive 10000 insertions")
    {
        GameManager gm;
        Entity first = gm.createEntity();
        gm.addComponent<Position>(first, 1.f, 2.f);
        Position *ptr = &gm.getComponent<Position>(first);
        std::vector<Entity> others;

        for (int i = 0; i < 10000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Position>(e, static_cast<float>(i), 0.f);
            others.push_back(e);
        }
        cr_assert(&gm.getComponent<Position>(first) == ptr);
        cr_assert(ptr->x == 1.f && ptr->y == 2.f);
        for (std::size_t i = 0; i < others.size(); i++)
            cr_assert(gm.getComponent<Position>(others[i]).x ==
                static_cast<float>(i));
    }

    Test(edge_cases, referencesSurviveRemovals,
        .description = "references survive removals")
    {
        GameManager gm;
        std::vector<Entity> entities;
        std::vector<Position *> ptrs;

        for (int i = 0; i < 5000; i++) {
            Entity e = gm.createEntity();
            entities.push_back(e);
            gm.addComponent<Position>(e, static_cast<float>(i), 0.f);
            ptrs.push_back(&gm.getComponent<Position>(e));
        }
        for (std::size_t i = 0; i < entities.size(); i += 2)
            gm.destroyEntity(entities[i]);
        for (std::size_t i = 1; i < entities.size(); i += 2) {
            cr_assert(&gm.getComponent<Position>(entities[i]) == ptrs[i]);
            cr_assert(ptrs[i]->x == static_cast<float>(i));
        }
        cr_assert(gm.getArray<Position>().nbComponent() == 2500);
    }

    Test(edge_cases, largeIdsGrowSparse,
        .description = "large entity ids grow the sparse array")
    {
        GameManager gm;
        std::vector<Entity> entities;

        entities.reserve(5000);
        for (int i = 0; i < 5000; i++)
            entities.push_back(gm.createEntity());
        gm.addComponent<Health>(entities.back(), 42);
        for (std::size_t i = 0; i + 1 < entities.size(); i++)
            cr_assert(!gm.hasComponent<Health>(entities[i]));
        cr_assert(gm.getComponent<Health>(entities.back()).hp == 42);
        gm.addComponent<Health>(entities.front(), 1);
        cr_assert(gm.getComponent<Health>(entities.front()).hp == 1);
        cr_assert(gm.getComponent<Health>(entities.back()).hp == 42);
        cr_assert(gm.getArray<Health>().nbComponent() == 2);
    }

    Test(edge_cases, denseIterationSeesEveryComponent,
        .description = "dense iteration sees every live component")
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
        for (const auto &pack : gm.getArray<Health>().getDense()) {
            sum += pack.component.hp;
            count++;
        }
        cr_assert(sum == expected);
        cr_assert(count == gm.getArray<Health>().nbComponent());
        cr_assert(count == 66);
    }

    Test(edge_cases, getArrayOnUnknownType,
        .description = "[design] getArray on a type never added")
    {
        GameManager gm;

        try {
            auto &array = gm.getArray<NeverAdded>();
            cr_assert(array.nbComponent() == 0);
        } catch (const RType::EngineException &) {
        } catch (const std::exception &e) {
            cr_assert_fail(
                "getArray<NeverAdded>() leaked a non engine exception: %s",
                e.what());
        }
    }

    Test(edge_cases, destructorsRun,
        .description = "destructors run on remove, destroy and shutdown")
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
            cr_assert(Tracked::alive == 3);
            gm.removeComponent<Tracked>(a);
            cr_assert(Tracked::alive == 2);
            gm.destroyEntity(b);
            cr_assert(Tracked::alive == 1);
            cr_assert_throw(gm.addComponent<Tracked>(c, 9),
                RType::AlreadyExistingComponent);
            cr_assert(Tracked::alive == 1);
        }
        cr_assert(Tracked::alive == 0);
    }

    Test(edge_cases, moveOnlyComponent, .description = "move-only components")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<MoveOnly>(e, std::make_unique<int>(42));
        cr_assert(gm.getComponent<MoveOnly>(e).ptr != nullptr);
        cr_assert(*gm.getComponent<MoveOnly>(e).ptr == 42);
        gm.destroyEntity(e);
    }

    Test(edge_cases, heapComponentsSurvive,
        .description = "heap-owning components survive insertions and removals")
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
                cr_assert(!gm.hasComponent<Name>(entities[i]));
            else
                cr_assert(
                    gm.getComponent<Name>(entities[i]).value == nameOf(i));
        }
    }

    Test(edge_cases, emptyTagComponent, .description = "empty tag components")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Player>(a);
        cr_assert(gm.hasComponent<Player>(a));
        cr_assert(!gm.hasComponent<Player>(b));
        cr_assert(gm.getArray<Player>().nbComponent() == 1);
        gm.removeComponent<Player>(a);
        cr_assert(!gm.hasComponent<Player>(a));
    }

    Test(edge_cases, addOnDestroyedEntityIsRejected,
        .description =
            "[design] adding a component to a destroyed entity is rejected")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.destroyEntity(e);
        cr_assert_throw(gm.addComponent<Health>(e, 1), RType::EngineException);
    }

    Test(edge_cases, randomOperationsMatchModel,
        .description = "20000 random operations match a reference model")
    {
        GameManager gm;
        std::mt19937 rng(42);
        std::vector<Entity> alive;
        std::vector<Entity> dead;
        std::map<std::size_t, int> health;
        std::map<std::size_t, float> position;

        auto verify = [&]() {
            int sum = 0;
            for (Entity e : alive) {
                cr_assert(
                    gm.hasComponent<Health>(e) == health.contains(indexOf(e)));
                cr_assert(gm.hasComponent<Position>(e) ==
                    position.contains(indexOf(e)));
                if (health.contains(indexOf(e)))
                    cr_assert(
                        gm.getComponent<Health>(e).hp == health[indexOf(e)]);
                if (position.contains(indexOf(e)))
                    cr_assert(
                        gm.getComponent<Position>(e).x == position[indexOf(e)]);
            }
            for (Entity e : dead) {
                cr_assert(!gm.hasComponent<Health>(e));
                cr_assert(!gm.hasComponent<Position>(e));
            }
            for (const auto &pack : gm.getArray<Health>().getDense())
                sum += pack.component.hp;
            for (const auto &[_, hp] : health)
                sum -= hp;
            cr_assert(sum == 0);
            cr_assert(gm.getArray<Health>().nbComponent() == health.size());
            cr_assert(gm.getArray<Position>().nbComponent() == position.size());
        };

        for (int i = 0; i < 20000; i++) {
            unsigned op = rng() % 100;
            if (alive.empty() || op < 15) {
                Entity e = gm.createEntity();
                cr_assert(!gm.hasComponent<Health>(e));
                cr_assert(!gm.hasComponent<Position>(e));
                std::erase(dead, e);
                alive.push_back(e);
                continue;
            }
            std::size_t idx = rng() % alive.size();
            Entity e = alive[idx];
            int value = static_cast<int>(rng() % 1000);
            if (op < 25) {
                gm.destroyEntity(e);
                alive.erase(alive.begin() + static_cast<std::ptrdiff_t>(idx));
                dead.push_back(e);
                health.erase(indexOf(e));
                position.erase(indexOf(e));
            } else if (op < 45) {
                if (health.contains(indexOf(e))) {
                    cr_assert_throw(gm.addComponent<Health>(e, value),
                        RType::AlreadyExistingComponent);
                } else {
                    gm.addComponent<Health>(e, value);
                    health[indexOf(e)] = value;
                }
            } else if (op < 60) {
                if (health.contains(indexOf(e))) {
                    gm.removeComponent<Health>(e);
                    health.erase(indexOf(e));
                } else {
                    cr_assert_throw(
                        gm.removeComponent<Health>(e), RType::NoSuchComponent);
                }
            } else if (op < 75) {
                if (position.contains(indexOf(e))) {
                    gm.removeComponent<Position>(e);
                    position.erase(indexOf(e));
                } else {
                    gm.addComponent<Position>(
                        e, static_cast<float>(value), 0.f);
                    position[indexOf(e)] = static_cast<float>(value);
                }
            } else {
                if (health.contains(indexOf(e))) {
                    gm.getComponent<Health>(e).hp += 1;
                    health[indexOf(e)] += 1;
                } else {
                    cr_assert_throw(
                        static_cast<void>(gm.getComponent<Health>(e)),
                        RType::NoSuchComponent);
                }
            }
            if (i % 1000 == 999)
                verify();
        }
        verify();
    }

} // namespace
