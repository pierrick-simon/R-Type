/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Recycling
*/

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

namespace {

    using RType::Entity;
    using RType::GameManager;
    using namespace Poc;

    void destroyedIdIsReused()
    {
        GameManager gm;
        gm.createEntity();
        Entity b = gm.createEntity();
        gm.createEntity();

        gm.destroyEntity(b);
        POC_CHECK(!gm.hasEntity(b));
        Entity reused = gm.createEntity();
        POC_CHECK(indexOf(reused) == indexOf(b));
        POC_CHECK(gm.hasEntity(reused));
    }

    void reusedIdsAreDistinct()
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
        POC_CHECK(created == destroyed);
        Entity fresh = gm.createEntity();
        POC_CHECK(!created.contains(indexOf(fresh)));
        POC_CHECK(indexOf(fresh) == 10);
    }

    void idsStayBoundedUnderChurn()
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
        POC_CHECK(maxIndex <= 10);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 9);
        for (std::size_t i = 0; i < alive.size(); i++)
            POC_CHECK(gm.getComponent<Health>(alive[i]).hp ==
                100000 - static_cast<int>(alive.size()) + static_cast<int>(i));
    }

    void recycledEntityStartsEmpty()
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
        POC_CHECK(indexOf(b) == indexOf(a));
        POC_CHECK(!gm.hasComponent<Position>(b));
        POC_CHECK(!gm.hasComponent<Health>(b));
        POC_CHECK(!gm.hasComponent<Player>(b));
        gm.addComponent<Health>(b, 42);
        POC_CHECK(gm.getComponent<Health>(b).hp == 42);
        POC_CHECK(gm.getComponent<Health>(other).hp == 7);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 2);
        POC_CHECK(gm.getArray<Position>().nbComponent() == 0);
    }

    void sameIdRecycledManyTimes()
    {
        GameManager gm;
        Entity current = gm.createEntity();
        const std::size_t slot = indexOf(current);

        gm.addComponent<Health>(current, 0);
        for (int i = 1; i <= 1000; i++) {
            gm.destroyEntity(current);
            current = gm.createEntity();
            POC_CHECK(indexOf(current) == slot);
            POC_CHECK(!gm.hasComponent<Health>(current));
            gm.addComponent<Health>(current, i);
        }
        POC_CHECK(gm.getComponent<Health>(current).hp == 1000);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void doubleDestroyDoesNotDuplicateId()
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
        POC_CHECK(indexOf(x) != indexOf(y));
        gm.addComponent<Health>(x, 1);
        gm.addComponent<Health>(y, 2);
        POC_CHECK(gm.getComponent<Health>(x).hp == 1);
        POC_CHECK(gm.getComponent<Health>(y).hp == 2);
    }

    void doubleDestroyInOneUpdateDoesNotDuplicateId()
    {
        GameManager gm;
        Entity enemy = gm.createEntity();

        for (int i = 0; i < 3; i++)
            gm.addComponent<Owned>(gm.createEntity(), enemy, i);
        gm.createSystem<Owned>([&gm](Owned &o) { gm.destroyEntity(o.owner); });
        gm.update();
        POC_CHECK(!gm.hasEntity(enemy));
        Entity x = gm.createEntity();
        Entity y = gm.createEntity();
        POC_CHECK(indexOf(x) == indexOf(enemy));
        POC_CHECK(indexOf(x) != indexOf(y));
    }

    void idDestroyedDuringUpdateIsNotReusedInSameFrame()
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
        POC_CHECK(indexOf(spawned) != indexOf(victim));
        POC_CHECK(victimAliveDuringFrame);
        POC_CHECK(!gm.hasEntity(victim));
        POC_CHECK(gm.hasEntity(spawned));
        POC_CHECK(indexOf(gm.createEntity()) == indexOf(victim));
    }

    void recycledEntityIsVisitedBySystems()
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
        POC_CHECK((seen == std::vector<int> {1, 2}));
    }

    void churnWithSystemsMatchesModel()
    {
        GameManager gm;
        std::vector<std::pair<Entity, int>> alive;
        std::size_t maxIndex = 0;

        gm.createSystem<Lifetime>([](Lifetime &l) { l.ticks--; });
        for (int tick = 0; tick < 500; tick++) {
            for (int i = 0; i < 3; i++) {
                Entity e = gm.createEntity();
                POC_CHECK(!gm.hasComponent<Lifetime>(e));
                POC_CHECK(!gm.hasComponent<Health>(e));
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
                POC_CHECK(gm.hasEntity(e));
                POC_CHECK(gm.getComponent<Health>(e).hp == born);
                POC_CHECK(gm.getComponent<Lifetime>(e).owner == e);
            }
            POC_CHECK(gm.getArray<Lifetime>().nbComponent() == alive.size());
        }
        POC_CHECK(maxIndex < 30);
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

    void recycledHandleDiffersFromOld()
    {
        Recycled r;

        POC_CHECK(indexOf(r.fresh) == indexOf(r.stale));
        POC_CHECK(!(r.fresh == r.stale));
    }

    void staleHandleIsNotAlive()
    {
        Recycled r;

        POC_CHECK(!r.gm.hasEntity(r.stale));
        POC_CHECK(r.gm.hasEntity(r.fresh));
    }

    void staleHandleHasNoComponent()
    {
        Recycled r;

        POC_CHECK(!r.gm.hasComponent<Health>(r.stale));
        POC_CHECK(r.gm.hasComponent<Health>(r.fresh));
    }

    void staleHandleCannotReadNewComponent()
    {
        Recycled r;

        POC_CHECK_THROWS(
            r.gm.getComponent<Health>(r.stale), RType::EngineException);
        POC_CHECK(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    void staleHandleCannotAddComponent()
    {
        Recycled r;

        POC_CHECK_THROWS(r.gm.addComponent<Position>(r.stale, 5.f, 5.f),
            RType::EngineException);
        POC_CHECK(!r.gm.hasComponent<Position>(r.fresh));
    }

    void staleHandleCannotRemoveComponent()
    {
        Recycled r;

        try {
            r.gm.removeComponent<Health>(r.stale);
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(r.gm.hasComponent<Health>(r.fresh));
        POC_CHECK(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    void staleHandleCannotDestroyNewEntity()
    {
        Recycled r;

        try {
            r.gm.destroyEntity(r.stale);
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(r.gm.hasEntity(r.fresh));
        POC_CHECK(r.gm.getComponent<Health>(r.fresh).hp == 99);
    }

    void storedTargetIsDetectedAsDead()
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
        POC_CHECK(hits == 1);
        gm.destroyEntity(target);
        Entity player = gm.createEntity();
        gm.addComponent<Health>(player, 3);
        gm.update();
        gm.update();
        POC_CHECK(hits == 1);
        POC_CHECK(gm.getComponent<Health>(player).hp == 3);
    }

    void everyOldGenerationIsDead()
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
            POC_CHECK(indexOf(old) == indexOf(current));
            POC_CHECK(!gm.hasEntity(old));
        }
        POC_CHECK(gm.hasEntity(current));
    }

    void highGenerationsStillIndexArrays()
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
        POC_CHECK(gm.hasComponent<Health>(current));
        POC_CHECK(gm.getComponent<Health>(current).hp == 1);
        POC_CHECK(gm.getComponent<Position>(current).y == 3.f);
        POC_CHECK(gm.hasComponent<Player>(current));
        POC_CHECK(gm.getComponent<Health>(keep).hp == 7);
        gm.removeComponent<Health>(current);
        POC_CHECK(!gm.hasComponent<Health>(current));
        gm.destroyEntity(current);
        POC_CHECK(gm.getArray<Position>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void staleHandleUsedDuringUpdate()
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
        POC_CHECK(gm.hasEntity(player));
        POC_CHECK(!gm.hasComponent<Damage>(player));
        POC_CHECK(gm.getComponent<Health>(player).hp == 3);
    }

} // namespace

namespace Poc {

    TestList recyclingTests()
    {
        return {
            {"a destroyed id is reused", destroyedIdIsReused},
            {"reused ids are distinct, then fresh ids resume",
                reusedIdsAreDistinct},
            {"ids stay bounded under 100000 create/destroy",
                idsStayBoundedUnderChurn},
            {"a recycled entity starts without components",
                recycledEntityStartsEmpty},
            {"the same id recycled 1000 times", sameIdRecycledManyTimes},
            {"a double destroy does not hand out the same id twice",
                doubleDestroyDoesNotDuplicateId},
            {"a double destroy in one update does not duplicate the id",
                doubleDestroyInOneUpdateDoesNotDuplicateId},
            {"an id destroyed during update is not reused in the same frame",
                idDestroyedDuringUpdateIsNotReusedInSameFrame},
            {"a recycled entity is visited by systems",
                recycledEntityIsVisitedBySystems},
            {"500 ticks of churn with systems match a model",
                churnWithSystemsMatchesModel},
            {"[generation] a recycled handle differs from the old one",
                recycledHandleDiffersFromOld},
            {"[generation] a stale handle is not alive", staleHandleIsNotAlive},
            {"[generation] a stale handle has no component",
                staleHandleHasNoComponent},
            {"[generation] a stale handle cannot read the new component",
                staleHandleCannotReadNewComponent},
            {"[generation] a stale handle cannot add a component",
                staleHandleCannotAddComponent},
            {"[generation] a stale handle cannot remove a component",
                staleHandleCannotRemoveComponent},
            {"[generation] a stale handle cannot destroy the new entity",
                staleHandleCannotDestroyNewEntity},
            {"[generation] a stored target is detected as dead",
                storedTargetIsDetectedAsDead},
            {"[generation] every old generation of a slot is dead",
                everyOldGenerationIsDead},
            {"[generation] high generations still index the arrays",
                highGenerationsStillIndexArrays},
            {"[generation] a stale handle used during update",
                staleHandleUsedDuringUpdate},
        };
    }

} // namespace Poc
