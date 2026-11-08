/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_recycling
*/

#include <algorithm>
#include <cstddef>
#include <set>
#include <type_traits>
#include <utility>
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

    struct Health {
        int hp;
    };

    struct Damage {
        int value;
    };

    struct Lifetime {
        RType::Entity owner;
        int ticks;
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

    using RType::Entity;
    using RType::GameManager;

    Test(recycling, destroyedIdIsReused,
        .description = "a destroyed id is reused")
    {
        GameManager gm;
        gm.createEntity();
        Entity b = gm.createEntity();
        gm.createEntity();

        gm.destroyEntity(b);
        cr_assert(!gm.hasEntity(b));
        Entity reused = gm.createEntity();
        cr_assert(indexOf(reused) == indexOf(b));
        cr_assert(gm.hasEntity(reused));
    }

    Test(recycling, reusedIdsAreDistinct,
        .description = "reused ids are distinct, then fresh ids resume")
    {
        GameManager gm;
        std::vector<Entity> entities;
        std::set<std::size_t> destroyed;
        std::set<std::size_t> created;

        entities.reserve(10);
        for (int i = 0; i < 10; i++)
            entities.push_back(gm.createEntity());
        for (int i = 0; i < 10; i += 3) {
            gm.destroyEntity(entities[i]);
            destroyed.insert(indexOf(entities[i]));
        }
        for (std::size_t i = 0; i < destroyed.size(); i++)
            created.insert(indexOf(gm.createEntity()));
        cr_assert(created == destroyed);
        Entity fresh = gm.createEntity();
        cr_assert(!created.contains(indexOf(fresh)));
        cr_assert(indexOf(fresh) == 10);
    }

    Test(recycling, idsStayBoundedUnderChurn,
        .description = "ids stay bounded under 100000 create/destroy")
    {
        GameManager gm;
        std::vector<Entity> alive;
        std::size_t maxIndex = 0;

        for (int i = 0; i < 100000; i++) {
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, i);
            maxIndex = std::max(maxIndex, indexOf(e));
            alive.push_back(e);
            if (alive.size() == 10) {
                gm.destroyEntity(alive.front());
                alive.erase(alive.begin());
            }
        }
        cr_assert(maxIndex <= 10);
        cr_assert(gm.getArray<Health>().nbComponent() == 9);
        for (std::size_t i = 0; i < alive.size(); i++)
            cr_assert(gm.getComponent<Health>(alive[i]).hp ==
                100000 - static_cast<int>(alive.size()) + static_cast<int>(i));
    }

    Test(recycling, recycledEntityStartsEmpty,
        .description = "a recycled entity starts without components")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity other = gm.createEntity();

        gm.addComponent<Position>(a, 1.f, 1.f);
        gm.addComponent<Health>(a, 5);
        gm.addComponent<Player>(a);
        gm.addComponent<Health>(other, 7);
        gm.destroyEntity(a);
        Entity b = gm.createEntity();
        cr_assert(indexOf(b) == indexOf(a));
        cr_assert(!gm.hasComponent<Position>(b));
        cr_assert(!gm.hasComponent<Health>(b));
        cr_assert(!gm.hasComponent<Player>(b));
        gm.addComponent<Health>(b, 42);
        cr_assert(gm.getComponent<Health>(b).hp == 42);
        cr_assert(gm.getComponent<Health>(other).hp == 7);
        cr_assert(gm.getArray<Health>().nbComponent() == 2);
        cr_assert(gm.getArray<Position>().nbComponent() == 0);
    }

    Test(recycling, sameIdRecycledManyTimes,
        .description = "the same id recycled 1000 times")
    {
        GameManager gm;
        Entity current = gm.createEntity();
        const std::size_t slot = indexOf(current);

        gm.addComponent<Health>(current, 0);
        for (int i = 1; i <= 1000; i++) {
            gm.destroyEntity(current);
            current = gm.createEntity();
            cr_assert(indexOf(current) == slot);
            cr_assert(!gm.hasComponent<Health>(current));
            gm.addComponent<Health>(current, i);
        }
        cr_assert(gm.getComponent<Health>(current).hp == 1000);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(recycling, doubleDestroyDoesNotDuplicateId,
        .description = "a double destroy does not hand out the same id twice")
    {
        GameManager gm;
        Entity a = gm.createEntity();

        gm.destroyEntity(a);
        try {
            gm.destroyEntity(a);
        } catch (const RType::EngineException &) {
        }
        Entity x = gm.createEntity();
        Entity y = gm.createEntity();
        cr_assert(indexOf(x) != indexOf(y));
        gm.addComponent<Health>(x, 1);
        gm.addComponent<Health>(y, 2);
        cr_assert(gm.getComponent<Health>(x).hp == 1);
        cr_assert(gm.getComponent<Health>(y).hp == 2);
    }

    Test(recycling, doubleDestroyInOneUpdateDoesNotDuplicateId,
        .description =
            "a double destroy in one update does not duplicate the id")
    {
        GameManager gm;
        Entity enemy = gm.createEntity();

        for (int i = 0; i < 3; i++)
            gm.addComponent<Owned>(gm.createEntity(), enemy, i);
        gm.createSystem<Owned>([&gm](Owned &o) { gm.destroyEntity(o.owner); });
        gm.update();
        cr_assert(!gm.hasEntity(enemy));
        Entity x = gm.createEntity();
        Entity y = gm.createEntity();
        cr_assert(indexOf(x) == indexOf(enemy));
        cr_assert(indexOf(x) != indexOf(y));
    }

    Test(recycling, idDestroyedDuringUpdateIsNotReusedInSameFrame,
        .description =
            "an id destroyed during update is not reused in the same frame")
    {
        GameManager gm;
        Entity victim = gm.createEntity();
        Entity spawned = victim;
        bool victimAliveDuringFrame = false;

        gm.addComponent<Owned>(victim, victim, 0);
        gm.createSystem<Owned>([&](Owned &o) {
            gm.destroyEntity(o.owner);
            spawned = gm.createEntity();
            victimAliveDuringFrame = gm.hasEntity(o.owner);
        });
        gm.update();
        cr_assert(indexOf(spawned) != indexOf(victim));
        cr_assert(victimAliveDuringFrame);
        cr_assert(!gm.hasEntity(victim));
        cr_assert(gm.hasEntity(spawned));
        cr_assert(indexOf(gm.createEntity()) == indexOf(victim));
    }

    Test(recycling, recycledEntityIsVisitedBySystems,
        .description = "a recycled entity is visited by systems")
    {
        GameManager gm;
        std::vector<int> seen;

        gm.createSystem<Health>([&seen](Health &h) { seen.push_back(h.hp); });
        Entity a = gm.createEntity();
        gm.addComponent<Health>(a, 1);
        gm.update();
        gm.destroyEntity(a);
        Entity b = gm.createEntity();
        gm.addComponent<Health>(b, 2);
        gm.update();
        cr_assert((seen == std::vector<int> {1, 2}));
    }

    Test(recycling, churnWithSystemsMatchesModel,
        .description = "500 ticks of churn with systems match a model")
    {
        GameManager gm;
        std::vector<std::pair<Entity, int>> alive;
        std::size_t maxIndex = 0;

        gm.createSystem<Lifetime>([](Lifetime &l) { l.ticks--; });
        for (int tick = 0; tick < 500; tick++) {
            for (int i = 0; i < 3; i++) {
                Entity e = gm.createEntity();
                cr_assert(!gm.hasComponent<Lifetime>(e));
                cr_assert(!gm.hasComponent<Health>(e));
                int life = 1 + (tick + i) % 7;
                gm.addComponent<Lifetime>(e, e, life);
                gm.addComponent<Health>(e, tick);
                alive.emplace_back(e, tick);
                maxIndex = std::max(maxIndex, indexOf(e));
            }
            gm.update();
            std::vector<Entity> expired;
            for (const auto &pack : gm.getArray<Lifetime>().getDense())
                if (pack.component.ticks <= 0)
                    expired.push_back(pack.component.owner);
            for (Entity e : expired) {
                gm.destroyEntity(e);
                std::erase_if(
                    alive, [e](const auto &p) { return p.first == e; });
            }
            for (const auto &[e, born] : alive) {
                cr_assert(gm.hasEntity(e));
                cr_assert(gm.getComponent<Health>(e).hp == born);
                cr_assert(gm.getComponent<Lifetime>(e).owner == e);
            }
            cr_assert(gm.getArray<Lifetime>().nbComponent() == alive.size());
        }
        cr_assert(maxIndex < 30);
    }

    struct Recycled {
        GameManager gm;
        Entity stale = gm.createEntity();
        Entity fresh = recycle();

        Entity recycle()
        {
            gm.addComponent<Health>(stale, 1);
            gm.addComponent<Position>(stale, 1.f, 1.f);
            gm.destroyEntity(stale);
            Entity e = gm.createEntity();
            gm.addComponent<Health>(e, 99);
            return e;
        }
    };

    Test(recycling, recycledHandleDiffersFromOld,
        .description =
            "[generation] a recycled handle differs from the old one")
    {
        Recycled r;

        cr_assert(indexOf(r.fresh) == indexOf(r.stale));
        cr_assert(!(r.fresh == r.stale));
    }

    Test(recycling, staleHandleIsNotAlive,
        .description = "[generation] a stale handle is not alive")
    {
        Recycled r;

        cr_assert(!r.gm.hasEntity(r.stale));
        cr_assert(r.gm.hasEntity(r.fresh));
    }

    Test(recycling, staleHandleHasNoComponent,
        .description = "[generation] a stale handle has no component")
    {
        Recycled r;

        cr_assert(!r.gm.hasComponent<Health>(r.stale));
        cr_assert(r.gm.hasComponent<Health>(r.fresh));
    }

    Test(recycling, staleHandleCannotReadNewComponent,
        .description =
            "[generation] a stale handle cannot read the new component")
    {
        Recycled r;

        cr_assert_throw(static_cast<void>(r.gm.getComponent<Health>(r.stale)),
            RType::EngineException);
        cr_assert(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    Test(recycling, staleHandleCannotAddComponent,
        .description = "[generation] a stale handle cannot add a component")
    {
        Recycled r;

        cr_assert_throw(r.gm.addComponent<Position>(r.stale, 5.f, 5.f),
            RType::EngineException);
        cr_assert(!r.gm.hasComponent<Position>(r.fresh));
    }

    Test(recycling, staleHandleCannotRemoveComponent,
        .description = "[generation] a stale handle cannot remove a component")
    {
        Recycled r;

        try {
            r.gm.removeComponent<Health>(r.stale);
        } catch (const RType::EngineException &) {
        }
        cr_assert(r.gm.hasComponent<Health>(r.fresh));
        cr_assert(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    Test(recycling, staleHandleCannotDestroyNewEntity,
        .description =
            "[generation] a stale handle cannot destroy the new entity")
    {
        Recycled r;

        try {
            r.gm.destroyEntity(r.stale);
        } catch (const RType::EngineException &) {
        }
        cr_assert(r.gm.hasEntity(r.fresh));
        cr_assert(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    Test(recycling, storedTargetIsDetectedAsDead,
        .description = "[generation] a stored target is detected as dead")
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();
        int hits = 0;

        gm.addComponent<Owned>(hunter, target, 0);
        gm.addComponent<Health>(target, 10);
        gm.createSystem<Owned>([&gm, &hits](Owned &o) {
            if (!gm.hasEntity(o.owner))
                return;
            gm.getComponent<Health>(o.owner).hp -= 1;
            hits++;
        });
        gm.update();
        cr_assert(hits == 1);
        gm.destroyEntity(target);
        Entity player = gm.createEntity();
        gm.addComponent<Health>(player, 3);
        gm.update();
        gm.update();
        cr_assert(hits == 1);
        cr_assert(gm.getComponent<Health>(player).hp == 3);
    }

    Test(recycling, everyOldGenerationIsDead,
        .description = "[generation] every old generation of a slot is dead")
    {
        GameManager gm;
        std::vector<Entity> generations;
        Entity current = gm.createEntity();

        for (int i = 0; i < 100; i++) {
            generations.push_back(current);
            gm.destroyEntity(current);
            current = gm.createEntity();
        }
        for (const Entity &old : generations) {
            cr_assert(indexOf(old) == indexOf(current));
            cr_assert(!gm.hasEntity(old));
        }
        cr_assert(gm.hasEntity(current));
    }

    Test(recycling, highGenerationsStillIndexArrays,
        .description = "[generation] high generations still index the arrays")
    {
        GameManager gm;
        Entity keep = gm.createEntity();
        Entity current = gm.createEntity();

        gm.addComponent<Health>(keep, 7);
        for (int i = 0; i < 50; i++) {
            gm.destroyEntity(current);
            current = gm.createEntity();
        }
        gm.addComponent<Health>(current, 1);
        gm.addComponent<Position>(current, 2.f, 3.f);
        gm.addComponent<Player>(current);
        cr_assert(gm.hasComponent<Health>(current));
        cr_assert(gm.getComponent<Health>(current).hp == 1);
        cr_assert(gm.getComponent<Position>(current).y == 3.f);
        cr_assert(gm.hasComponent<Player>(current));
        cr_assert(gm.getComponent<Health>(keep).hp == 7);
        gm.removeComponent<Health>(current);
        cr_assert(!gm.hasComponent<Health>(current));
        gm.destroyEntity(current);
        cr_assert(gm.getArray<Position>().nbComponent() == 0);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(recycling, staleHandleUsedDuringUpdate,
        .description = "[generation] a stale handle used during update")
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();

        gm.addComponent<Owned>(hunter, target, 0);
        gm.destroyEntity(target);
        Entity player = gm.createEntity();
        gm.addComponent<Health>(player, 3);
        gm.createSystem<Owned>([&gm](Owned &o) {
            gm.destroyEntity(o.owner);
            gm.addComponent<Damage>(o.owner, 1);
        });
        try {
            gm.update();
        } catch (const RType::EngineException &) {
        }
        cr_assert(gm.hasEntity(player));
        cr_assert(!gm.hasComponent<Damage>(player));
        cr_assert(gm.getComponent<Health>(player).hp == 3);
    }

} // namespace
