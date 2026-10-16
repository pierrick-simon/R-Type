/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Tricky
*/

#include <algorithm>
#include <stdexcept>
#include <string>
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

    struct Fragile {
        int value;

        explicit Fragile(int v) :
            value(v)
        {
            if (v < 0)
                throw std::runtime_error("negative fragile");
        }
    };

    struct Link {
        GameManager *gm = nullptr;
        Entity child;

        Link(GameManager *manager, Entity target) :
            gm(manager), child(target)
        {
        }

        Link(const Link &) = delete;
        Link &operator=(const Link &) = delete;

        Link(Link &&other) noexcept :
            gm(std::exchange(other.gm, nullptr)), child(other.child)
        {
        }

        Link &operator=(Link &&other) noexcept
        {
            gm = std::exchange(other.gm, nullptr);
            child = other.child;
            return *this;
        }

        ~Link()
        {
            if (gm != nullptr && gm->hasEntity(child))
                gm->destroyEntity(child);
        }
    };

    struct Spawner {
        GameManager *gm = nullptr;
        int *spawned = nullptr;

        Spawner(GameManager *manager, int *counter) :
            gm(manager), spawned(counter)
        {
        }

        Spawner(const Spawner &) = delete;
        Spawner &operator=(const Spawner &) = delete;

        Spawner(Spawner &&other) noexcept :
            gm(std::exchange(other.gm, nullptr)), spawned(other.spawned)
        {
        }

        Spawner &operator=(Spawner &&other) noexcept
        {
            gm = std::exchange(other.gm, nullptr);
            spawned = other.spawned;
            return *this;
        }

        ~Spawner()
        {
            if (gm == nullptr)
                return;
            Entity e = gm->createEntity();
            gm->addComponent<Health>(e, 1);
            (*spawned)++;
        }
    };

    void throwingConstructorLeavesNothing()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        POC_CHECK_THROWS(gm.addComponent<Fragile>(e, -1), std::runtime_error);
        POC_CHECK(!gm.hasComponent<Fragile>(e));
        gm.addComponent<Health>(e, 3);
        POC_CHECK(gm.getComponent<Health>(e).hp == 3);
        gm.addComponent<Fragile>(e, 4);
        POC_CHECK(gm.getComponent<Fragile>(e).value == 4);
    }

    void throwingConstructorDuringUpdate()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Entity victim = gm.createEntity();
        bool done = false;

        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([&](Health &) {
            if (done)
                return;
            done = true;
            gm.destroyEntity(victim);
            gm.addComponent<Fragile>(e, -1);
        });
        POC_CHECK_THROWS(gm.update(), std::runtime_error);
        POC_CHECK(!gm.hasEntity(victim));
        POC_CHECK(!gm.hasComponent<Fragile>(e));
        POC_CHECK_NOTHROW(gm.update());
        gm.addComponent<Fragile>(e, 2);
        POC_CHECK(gm.getComponent<Fragile>(e).value == 2);
    }

    void failedAddDoesNotLeak()
    {
        const int before = Tracked::alive;
        {
            GameManager gm;
            Entity dead = gm.createEntity();

            gm.destroyEntity(dead);
            try {
                gm.addComponent<Tracked>(dead, 1);
            } catch (const RType::EngineException &) {
            }
            POC_CHECK(Tracked::alive == before);
        }
        POC_CHECK(Tracked::alive == before);
    }

    void failedAddDuringUpdateDoesNotLeak()
    {
        const int before = Tracked::alive;
        {
            GameManager gm;
            Entity target = gm.createEntity();
            Entity hunter = gm.createEntity();

            gm.addComponent<Owned>(hunter, target, 0);
            gm.createSystem<Owned>([&gm](Owned &o) {
                gm.destroyEntity(o.owner);
                gm.addComponent<Tracked>(o.owner, 7);
            });
            try {
                gm.update();
            } catch (const RType::EngineException &) {
            }
            POC_CHECK(!gm.hasEntity(target));
            POC_CHECK(Tracked::alive == before);
        }
        POC_CHECK(Tracked::alive == before);
    }

    void addThenDestroyInSameFrame()
    {
        const int before = Tracked::alive;
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();

        gm.addComponent<Owned>(hunter, target, 0);
        gm.createSystem<Owned>([&gm](Owned &o) {
            gm.addComponent<Tracked>(o.owner, 1);
            gm.addComponent<Health>(o.owner, 1);
            gm.destroyEntity(o.owner);
        });
        gm.update();
        POC_CHECK(!gm.hasEntity(target));
        POC_CHECK(gm.getArray<Health>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Tracked>().nbComponent() == 0);
        POC_CHECK(Tracked::alive == before);
        Entity recycled = gm.createEntity();
        POC_CHECK(indexOf(recycled) == indexOf(target));
        POC_CHECK(!gm.hasComponent<Health>(recycled));
        POC_CHECK(!gm.hasComponent<Tracked>(recycled));
    }

    void removeThenReAddInSameFrame()
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();

        gm.addComponent<Health>(target, 1);
        gm.addComponent<Owned>(hunter, target, 0);
        gm.createSystem<Owned>([&gm](Owned &o) {
            int hp = gm.getComponent<Health>(o.owner).hp;
            gm.removeComponent<Health>(o.owner);
            gm.addComponent<Health>(o.owner, hp + 10);
        });
        gm.update();
        gm.update();
        POC_CHECK(gm.getComponent<Health>(target).hp == 21);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void addTwiceInSameFrameKeepsTheRestOfTheFrame()
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();
        Entity victim = gm.createEntity();

        gm.addComponent<Owned>(hunter, target, 0);
        gm.createSystem<Owned>([&](Owned &o) {
            gm.addComponent<Health>(o.owner, 1);
            gm.addComponent<Health>(o.owner, 2);
            gm.destroyEntity(victim);
        });
        POC_CHECK_THROWS(gm.update(), RType::EngineException);
        POC_CHECK(gm.getComponent<Health>(target).hp == 1);
        POC_CHECK(!gm.hasEntity(victim));
        gm.destroyEntity(target);
        POC_CHECK(!gm.hasEntity(target));
    }

    void systemErrorWinsOverQueuedErrors()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Entity dead = gm.createEntity();
        Entity victim = gm.createEntity();
        std::string message;

        gm.destroyEntity(dead);
        gm.addComponent<Health>(e, 1);
        gm.createSystem<Health>([&](Health &) {
            gm.addComponent<Health>(dead, 1);
            gm.destroyEntity(victim);
            throw std::runtime_error("boom");
        });
        try {
            gm.update();
        } catch (const std::runtime_error &error) {
            message = error.what();
        } catch (const std::exception &error) {
            message = std::string("wrong exception: ") + error.what();
        }
        POC_CHECK(message == "boom");
        POC_CHECK(!gm.hasEntity(victim));
    }

    void nestedUpdateIsIgnored()
    {
        GameManager gm;
        std::vector<Entity> targets;
        int runs = 0;
        bool aliveAfterNested = true;

        for (int i = 0; i < 3; i++) {
            Entity target = gm.createEntity();
            gm.addComponent<Owned>(gm.createEntity(), target, i);
            targets.push_back(target);
        }
        gm.createSystem<Owned>([&](Owned &o) {
            runs++;
            gm.destroyEntity(o.owner);
            gm.update();
            aliveAfterNested = aliveAfterNested && gm.hasEntity(o.owner);
        });
        gm.update();
        POC_CHECK(runs == 3);
        POC_CHECK(aliveAfterNested);
        for (const Entity &target : targets)
            POC_CHECK(!gm.hasEntity(target));
    }

    void componentReadableUntilEndOfFrame()
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();
        int read = 0;

        gm.addComponent<Health>(target, 42);
        gm.addComponent<Owned>(hunter, target, 0);
        gm.addComponent<Damage>(hunter, 0);
        gm.createSystem<Owned>([&gm](Owned &o) { gm.destroyEntity(o.owner); });
        gm.createSystem<Damage>(
            [&](Damage &) { read = gm.getComponent<Health>(target).hp; });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(read == 42);
        POC_CHECK(!gm.hasEntity(target));
    }

    void throwingFrameStillDestroysEverything()
    {
        const int before = Tracked::alive;
        GameManager gm;
        std::vector<Entity> entities;

        for (int i = 0; i < 20; i++) {
            entities.push_back(gm.createEntity());
            gm.addComponent<Tracked>(entities.back(), i);
            gm.addComponent<Owned>(entities.back(), entities.back(), i);
        }
        gm.createSystem<Owned>([&gm](Owned &o) {
            gm.destroyEntity(o.owner);
            if (o.value == 19)
                throw std::runtime_error("last one");
        });
        POC_CHECK_THROWS(gm.update(), std::runtime_error);
        for (const Entity &e : entities)
            POC_CHECK(!gm.hasEntity(e));
        POC_CHECK(gm.getArray<Owned>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Tracked>().nbComponent() == 0);
        POC_CHECK(Tracked::alive == before);
    }

    void systemCreatedDuringUpdateStartsNextFrame()
    {
        GameManager gm;
        int runs = 0;
        bool created = false;

        for (int i = 0; i < 4; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<Health>([&](Health &) {
            if (created)
                return;
            created = true;
            gm.createSystem<Health>([&runs](Health &) { runs++; });
        });
        gm.update();
        POC_CHECK(runs == 0);
        gm.update();
        POC_CHECK(runs == 4);
    }

    void staleSystemHandleDestroyedTwice()
    {
        GameManager gm;
        int runs = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto system = gm.createSystem<Health>([](Health &) {});
        gm.destroySystem(system);
        try {
            gm.destroySystem(system);
        } catch (const RType::EngineException &) {
        }
        gm.createSystem<Health>([&runs](Health &) { runs++; });
        gm.update();
        POC_CHECK(runs == 1);
    }

    void twoSystemsDestroySameSystemInOneFrame()
    {
        GameManager gm;
        int targetRuns = 0;
        int afterRuns = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto target =
            gm.createSystem<Health>([&targetRuns](Health &) { targetRuns++; });
        gm.createSystem<Health>([&](Health &) { gm.destroySystem(target); });
        gm.createSystem<Health>([&](Health &) { gm.destroySystem(target); });
        gm.createSystem<Health>([&afterRuns](Health &) { afterRuns++; });
        try {
            gm.update();
        } catch (const RType::EngineException &) {
        }
        gm.update();
        POC_CHECK(targetRuns == 1);
        POC_CHECK(afterRuns == 2);
    }

    void hasSystemFollowsCreateAndDestroy()
    {
        GameManager gm;
        auto a = gm.createSystem<Health>([](Health &) {});
        auto b = gm.createSystem<Health>([](Health &) {});

        POC_CHECK(a != b);
        POC_CHECK(gm.hasSystem(a));
        POC_CHECK(gm.hasSystem(b));
        gm.destroySystem(a);
        POC_CHECK(!gm.hasSystem(a));
        POC_CHECK(gm.hasSystem(b));
    }

    void staleSystemIdDoesNotHitANewSystem()
    {
        GameManager gm;
        int runs = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto stale = gm.createSystem<Health>([](Health &) {});
        gm.destroySystem(stale);
        auto fresh = gm.createSystem<Health>([&runs](Health &) { runs++; });
        POC_CHECK(fresh != stale);
        try {
            gm.destroySystem(stale);
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(gm.hasSystem(fresh));
        gm.update();
        POC_CHECK(runs == 1);
    }

    void systemIdsAreNeverReused()
    {
        GameManager gm;
        std::vector<RType::SystemId> ids;

        for (int i = 0; i < 1000; i++) {
            auto id = gm.createSystem<Health>([](Health &) {});
            POC_CHECK(std::ranges::find(ids, id) == ids.end());
            ids.push_back(id);
            gm.destroySystem(id);
        }
        for (auto id : ids)
            POC_CHECK(!gm.hasSystem(id));
    }

    void systemDestroyedDuringUpdateExistsUntilEndOfFrame()
    {
        GameManager gm;
        bool existedDuringFrame = false;
        RType::SystemId self = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        self = gm.createSystem<Health>([&](Health &) {
            gm.destroySystem(self);
            existedDuringFrame = gm.hasSystem(self);
        });
        gm.update();
        POC_CHECK(existedDuringFrame);
        POC_CHECK(!gm.hasSystem(self));
    }

    void destructorDestroysAnotherEntity()
    {
        GameManager gm;
        Entity parent = gm.createEntity();
        Entity child = gm.createEntity();

        gm.addComponent<Health>(child, 5);
        gm.addComponent<Link>(parent, &gm, child);
        gm.destroyEntity(parent);
        POC_CHECK(!gm.hasEntity(parent));
        POC_CHECK(!gm.hasEntity(child));
        POC_CHECK(gm.getArray<Health>().nbComponent() == 0);
    }

    void destructorDestroysAChainOfEntities()
    {
        GameManager gm;
        std::vector<Entity> chain;

        chain.reserve(50);
        for (int i = 0; i < 50; i++)
            chain.push_back(gm.createEntity());
        for (std::size_t i = 0; i + 1 < chain.size(); i++)
            gm.addComponent<Link>(chain[i], &gm, chain[i + 1]);
        gm.addComponent<Health>(chain.back(), 1);
        gm.destroyEntity(chain.front());
        for (const Entity &e : chain)
            POC_CHECK(!gm.hasEntity(e));
        POC_CHECK(gm.getArray<Link>().nbComponent() == 0);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 0);
    }

    void destructorDestroysDuringEndOfFrameFlush()
    {
        GameManager gm;
        Entity parent = gm.createEntity();
        Entity child = gm.createEntity();
        Entity bystander = gm.createEntity();

        gm.addComponent<Link>(parent, &gm, child);
        gm.addComponent<Health>(child, 1);
        gm.addComponent<Health>(bystander, 2);
        gm.addComponent<Owned>(bystander, parent, 0);
        gm.createSystem<Owned>([&gm](Owned &o) { gm.destroyEntity(o.owner); });
        gm.update();
        POC_CHECK(!gm.hasEntity(parent));
        POC_CHECK(!gm.hasEntity(child));
        POC_CHECK(gm.getComponent<Health>(bystander).hp == 2);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void removingALinkDestroysTheChild()
    {
        GameManager gm;
        Entity parent = gm.createEntity();
        Entity child = gm.createEntity();

        gm.addComponent<Link>(parent, &gm, child);
        gm.removeComponent<Link>(parent);
        POC_CHECK(gm.hasEntity(parent));
        POC_CHECK(!gm.hasEntity(child));
    }

    void destructorCreatesEntities()
    {
        GameManager gm;
        int spawned = 0;
        std::vector<Entity> spawners;

        for (int i = 0; i < 10; i++) {
            spawners.push_back(gm.createEntity());
            gm.addComponent<Spawner>(spawners.back(), &gm, &spawned);
        }
        for (const Entity &e : spawners)
            gm.destroyEntity(e);
        POC_CHECK(spawned == 10);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 10);
        POC_CHECK(gm.getArray<Spawner>().nbComponent() == 0);
    }

    void forgedHugeHandle()
    {
        GameManager gm;
        Entity real = gm.createEntity();
        Entity forged(1'000'000'000);

        gm.addComponent<Health>(real, 1);
        POC_CHECK(!gm.hasEntity(forged));
        POC_CHECK(!gm.hasComponent<Health>(forged));
        POC_CHECK_THROWS(
            gm.getComponent<Health>(forged), RType::EngineException);
        POC_CHECK_THROWS(
            gm.addComponent<Health>(forged, 1), RType::EngineException);
        POC_CHECK_THROWS(
            gm.removeComponent<Health>(forged), RType::EngineException);
        POC_CHECK_THROWS(gm.removeComponents(forged), RType::EngineException);
        POC_CHECK_NOTHROW(gm.destroyEntity(forged));
        POC_CHECK(gm.getComponent<Health>(real).hp == 1);
    }

    void handleFromTheFuture()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Entity future(indexOf(e), 3);

        gm.addComponent<Health>(e, 1);
        POC_CHECK(!gm.hasEntity(future));
        POC_CHECK(!gm.hasComponent<Health>(future));
        POC_CHECK_THROWS(gm.addComponent<Position>(future, 0.f, 0.f),
            RType::EngineException);
        try {
            gm.removeComponent<Health>(future);
        } catch (const RType::EngineException &) {
        }
        try {
            gm.removeComponents(future);
        } catch (const RType::EngineException &) {
        }
        gm.destroyEntity(future);
        POC_CHECK(gm.hasEntity(e));
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
        POC_CHECK(!gm.hasComponent<Position>(e));
    }

    void staleHandleCannotRemoveEveryComponent()
    {
        GameManager gm;
        Entity stale = gm.createEntity();

        gm.destroyEntity(stale);
        Entity fresh = gm.createEntity();
        gm.addComponent<Health>(fresh, 9);
        gm.addComponent<Position>(fresh, 1.f, 2.f);
        try {
            gm.removeComponents(stale);
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(gm.getComponent<Health>(fresh).hp == 9);
        POC_CHECK(gm.hasComponent<Position>(fresh));
    }

    void staleHandleRemovesEverythingDuringUpdate()
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();

        gm.addComponent<Owned>(hunter, target, 0);
        gm.destroyEntity(target);
        Entity player = gm.createEntity();
        gm.addComponent<Health>(player, 3);
        gm.addComponent<Player>(player);
        gm.createSystem<Owned>(
            [&gm](Owned &o) { gm.removeComponents(o.owner); });
        try {
            gm.update();
        } catch (const RType::EngineException &) {
        }
        POC_CHECK(gm.getComponent<Health>(player).hp == 3);
        POC_CHECK(gm.hasComponent<Player>(player));
    }

    void removeEverythingThenDestroyInSameFrame()
    {
        GameManager gm;
        Entity target = gm.createEntity();
        Entity hunter = gm.createEntity();

        gm.addComponent<Health>(target, 1);
        gm.addComponent<Owned>(hunter, target, 0);
        gm.createSystem<Owned>([&gm](Owned &o) {
            gm.destroyEntity(o.owner);
            gm.removeComponents(o.owner);
        });
        POC_CHECK_THROWS(gm.update(), RType::NoSuchEntity);
        POC_CHECK(!gm.hasEntity(target));
        Entity fresh = gm.createEntity();
        POC_CHECK(indexOf(fresh) == indexOf(target));
        POC_CHECK(!gm.hasComponent<Health>(fresh));
    }

} // namespace

namespace Poc {

    TestList trickyTests()
    {
        return {
            {"[throw] a throwing component constructor leaves nothing",
                throwingConstructorLeavesNothing},
            {"[throw] a throwing component constructor during update",
                throwingConstructorDuringUpdate},
            {"[leak] a rejected add does not leak the component",
                failedAddDoesNotLeak},
            {"[leak] a rejected add during update does not leak",
                failedAddDuringUpdateDoesNotLeak},
            {"[frame] add then destroy in the same frame",
                addThenDestroyInSameFrame},
            {"[frame] remove then re-add in the same frame",
                removeThenReAddInSameFrame},
            {"[frame] a duplicate add does not cancel the rest of the frame",
                addTwiceInSameFrameKeepsTheRestOfTheFrame},
            {"[frame] the system error wins over the queued errors",
                systemErrorWinsOverQueuedErrors},
            {"[frame] a nested update is ignored", nestedUpdateIsIgnored},
            {"[frame] a destroyed entity is readable until the end of frame",
                componentReadableUntilEndOfFrame},
            {"[frame] a throwing frame still applies every destroy",
                throwingFrameStillDestroysEverything},
            {"[frame] destroy then removeComponents in the same frame",
                removeEverythingThenDestroyInSameFrame},
            {"[unsupported] a system created during update starts next frame",
                systemCreatedDuringUpdateStartsNextFrame},
            {"[stale system] destroying a system twice",
                staleSystemHandleDestroyedTwice},
            {"[stale system] two systems destroy the same system in a frame",
                twoSystemsDestroySameSystemInOneFrame},
            {"[stale system] hasSystem follows create and destroy",
                hasSystemFollowsCreateAndDestroy},
            {"[stale system] a stale id does not hit a new system",
                staleSystemIdDoesNotHitANewSystem},
            {"[stale system] ids are never reused", systemIdsAreNeverReused},
            {"[stale system] a system destroyed during update exists until "
             "end of frame",
                systemDestroyedDuringUpdateExistsUntilEndOfFrame},
            {"[unsupported] [reentrancy] a component destructor destroys "
             "another entity",
                destructorDestroysAnotherEntity},
            {"[unsupported] [reentrancy] a component destructor destroys a "
             "chain of 50",
                destructorDestroysAChainOfEntities},
            {"[unsupported] [reentrancy] a component destructor during the "
             "frame flush",
                destructorDestroysDuringEndOfFrameFlush},
            {"[unsupported] [reentrancy] removing a component destroys another "
             "entity",
                removingALinkDestroysTheChild},
            {"[unsupported] [reentrancy] a component destructor creates "
             "entities",
                destructorCreatesEntities},
            {"[forged] a handle with a huge index", forgedHugeHandle},
            {"[forged] a handle from a future generation", handleFromTheFuture},
            {"[generation] a stale handle cannot removeComponents",
                staleHandleCannotRemoveEveryComponent},
            {"[generation] a stale removeComponents during update",
                staleHandleRemovesEverythingDuringUpdate},
        };
    }

} // namespace Poc
