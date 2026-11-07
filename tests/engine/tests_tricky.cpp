/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_tricky
*/

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
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

    using RType::Entity;
    using RType::GameManager;

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

    Test(tricky, throwingConstructorLeavesNothing,
        .description =
            "[throw] a throwing component constructor leaves nothing")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        cr_assert_throw(gm.addComponent<Fragile>(e, -1), std::runtime_error);
        cr_assert(!gm.hasComponent<Fragile>(e));
        gm.addComponent<Health>(e, 3);
        cr_assert(gm.getComponent<Health>(e).hp == 3);
        gm.addComponent<Fragile>(e, 4);
        cr_assert(gm.getComponent<Fragile>(e).value == 4);
    }

    Test(tricky, throwingConstructorDuringUpdate,
        .description = "[throw] a throwing component constructor during update")
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
        cr_assert_throw(gm.update(), std::runtime_error);
        cr_assert(!gm.hasEntity(victim));
        cr_assert(!gm.hasComponent<Fragile>(e));
        cr_assert_no_throw(gm.update(), std::exception);
        gm.addComponent<Fragile>(e, 2);
        cr_assert(gm.getComponent<Fragile>(e).value == 2);
    }

    Test(tricky, failedAddDoesNotLeak,
        .description = "[leak] a rejected add does not leak the component")
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
            cr_assert(Tracked::alive == before);
        }
        cr_assert(Tracked::alive == before);
    }

    Test(tricky, failedAddDuringUpdateDoesNotLeak,
        .description = "[leak] a rejected add during update does not leak")
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
            cr_assert(!gm.hasEntity(target));
            cr_assert(Tracked::alive == before);
        }
        cr_assert(Tracked::alive == before);
    }

    Test(tricky, addThenDestroyInSameFrame,
        .description = "[frame] add then destroy in the same frame")
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
        cr_assert(!gm.hasEntity(target));
        cr_assert(gm.getArray<Health>().nbComponent() == 0);
        cr_assert(gm.getArray<Tracked>().nbComponent() == 0);
        cr_assert(Tracked::alive == before);
        Entity recycled = gm.createEntity();
        cr_assert(indexOf(recycled) == indexOf(target));
        cr_assert(!gm.hasComponent<Health>(recycled));
        cr_assert(!gm.hasComponent<Tracked>(recycled));
    }

    Test(tricky, removeThenReAddInSameFrame,
        .description = "[frame] remove then re-add in the same frame")
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
        cr_assert(gm.getComponent<Health>(target).hp == 21);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(tricky, addTwiceInSameFrameKeepsTheRestOfTheFrame,
        .description =
            "[frame] a duplicate add does not cancel the rest of the frame")
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
        cr_assert_throw(gm.update(), RType::EngineException);
        cr_assert(gm.getComponent<Health>(target).hp == 1);
        cr_assert(!gm.hasEntity(victim));
        gm.destroyEntity(target);
        cr_assert(!gm.hasEntity(target));
    }

    Test(tricky, systemErrorWinsOverQueuedErrors,
        .description = "[frame] the system error wins over the queued errors")
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
        cr_assert(message == "boom");
        cr_assert(!gm.hasEntity(victim));
    }

    Test(tricky, nestedUpdateIsIgnored,
        .description = "[frame] a nested update is ignored")
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
        cr_assert(runs == 3);
        cr_assert(aliveAfterNested);
        for (const Entity &target : targets)
            cr_assert(!gm.hasEntity(target));
    }

    Test(tricky, componentReadableUntilEndOfFrame,
        .description =
            "[frame] a destroyed entity is readable until the end of frame")
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
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(read == 42);
        cr_assert(!gm.hasEntity(target));
    }

    Test(tricky, throwingFrameStillDestroysEverything,
        .description = "[frame] a throwing frame still applies every destroy")
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
        cr_assert_throw(gm.update(), std::runtime_error);
        for (const Entity &e : entities)
            cr_assert(!gm.hasEntity(e));
        cr_assert(gm.getArray<Owned>().nbComponent() == 0);
        cr_assert(gm.getArray<Tracked>().nbComponent() == 0);
        cr_assert(Tracked::alive == before);
    }

    Test(tricky, systemCreatedDuringUpdateStartsNextFrame, .disabled = true,
        .description = "a system created during update starts next frame")
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
        cr_assert(runs == 0);
        gm.update();
        cr_assert(runs == 4);
    }

    Test(tricky, staleSystemHandleDestroyedTwice,
        .description = "[stale system] destroying a system twice")
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
        cr_assert(runs == 1);
    }

    Test(tricky, twoSystemsDestroySameSystemInOneFrame,
        .description =
            "[stale system] two systems destroy the same system in a frame")
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
        cr_assert(targetRuns == 1);
        cr_assert(afterRuns == 2);
    }

    Test(tricky, hasSystemFollowsCreateAndDestroy,
        .description = "[stale system] hasSystem follows create and destroy")
    {
        GameManager gm;
        auto a = gm.createSystem<Health>([](Health &) {});
        auto b = gm.createSystem<Health>([](Health &) {});

        cr_assert(a != b);
        cr_assert(gm.hasSystem(a));
        cr_assert(gm.hasSystem(b));
        gm.destroySystem(a);
        cr_assert(!gm.hasSystem(a));
        cr_assert(gm.hasSystem(b));
    }

    Test(tricky, staleSystemIdDoesNotHitANewSystem,
        .description = "[stale system] a stale id does not hit a new system")
    {
        GameManager gm;
        int runs = 0;

        gm.addComponent<Health>(gm.createEntity(), 1);
        auto stale = gm.createSystem<Health>([](Health &) {});
        gm.destroySystem(stale);
        auto fresh = gm.createSystem<Health>([&runs](Health &) { runs++; });
        cr_assert(fresh != stale);
        try {
            gm.destroySystem(stale);
        } catch (const RType::EngineException &) {
        }
        cr_assert(gm.hasSystem(fresh));
        gm.update();
        cr_assert(runs == 1);
    }

    Test(tricky, systemIdsAreNeverReused,
        .description = "[stale system] ids are never reused")
    {
        GameManager gm;
        std::vector<RType::SystemId> ids;

        for (int i = 0; i < 1000; i++) {
            auto id = gm.createSystem<Health>([](Health &) {});
            cr_assert(std::ranges::find(ids, id) == ids.end());
            ids.push_back(id);
            gm.destroySystem(id);
        }
        for (auto id : ids)
            cr_assert(!gm.hasSystem(id));
    }

    Test(tricky, systemDestroyedDuringUpdateExistsUntilEndOfFrame,
        .description = "[stale system] a system destroyed during update exists "
                       "until end of frame")
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
        cr_assert(existedDuringFrame);
        cr_assert(!gm.hasSystem(self));
    }

    Test(tricky, destructorDestroysAnotherEntity, .disabled = true,
        .description =
            "[reentrancy] a component destructor destroys another entity")
    {
        GameManager gm;
        Entity parent = gm.createEntity();
        Entity child = gm.createEntity();

        gm.addComponent<Health>(child, 5);
        gm.addComponent<Link>(parent, &gm, child);
        gm.destroyEntity(parent);
        cr_assert(!gm.hasEntity(parent));
        cr_assert(!gm.hasEntity(child));
        cr_assert(gm.getArray<Health>().nbComponent() == 0);
    }

    Test(tricky, destructorDestroysAChainOfEntities, .disabled = true,
        .description =
            "[reentrancy] a component destructor destroys a chain of 50")
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
            cr_assert(!gm.hasEntity(e));
        cr_assert(gm.getArray<Link>().nbComponent() == 0);
        cr_assert(gm.getArray<Health>().nbComponent() == 0);
    }

    Test(tricky, destructorDestroysDuringEndOfFrameFlush, .disabled = true,
        .description =
            "[reentrancy] a component destructor during the frame flush")
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
        cr_assert(!gm.hasEntity(parent));
        cr_assert(!gm.hasEntity(child));
        cr_assert(gm.getComponent<Health>(bystander).hp == 2);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(tricky, removingALinkDestroysTheChild, .disabled = true,
        .description =
            "[reentrancy] removing a component destroys another entity")
    {
        GameManager gm;
        Entity parent = gm.createEntity();
        Entity child = gm.createEntity();

        gm.addComponent<Link>(parent, &gm, child);
        gm.removeComponent<Link>(parent);
        cr_assert(gm.hasEntity(parent));
        cr_assert(!gm.hasEntity(child));
    }

    Test(tricky, destructorCreatesEntities, .disabled = true,
        .description = "[reentrancy] a component destructor creates entities")
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
        cr_assert(spawned == 10);
        cr_assert(gm.getArray<Health>().nbComponent() == 10);
        cr_assert(gm.getArray<Spawner>().nbComponent() == 0);
    }

    Test(tricky, forgedHugeHandle,
        .description = "[forged] a handle with a huge index")
    {
        GameManager gm;
        Entity real = gm.createEntity();
        Entity forged(1'000'000'000);

        gm.addComponent<Health>(real, 1);
        cr_assert(!gm.hasEntity(forged));
        cr_assert(!gm.hasComponent<Health>(forged));
        cr_assert_throw(static_cast<void>(gm.getComponent<Health>(forged)),
            RType::EngineException);
        cr_assert_throw(
            gm.addComponent<Health>(forged, 1), RType::EngineException);
        cr_assert_throw(
            gm.removeComponent<Health>(forged), RType::EngineException);
        cr_assert_throw(gm.removeComponents(forged), RType::EngineException);
        cr_assert_no_throw(gm.destroyEntity(forged), std::exception);
        cr_assert(gm.getComponent<Health>(real).hp == 1);
    }

    Test(tricky, handleFromTheFuture,
        .description = "[forged] a handle from a future generation")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Entity future(indexOf(e), 3);

        gm.addComponent<Health>(e, 1);
        cr_assert(!gm.hasEntity(future));
        cr_assert(!gm.hasComponent<Health>(future));
        cr_assert_throw(gm.addComponent<Position>(future, 0.f, 0.f),
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
        cr_assert(gm.hasEntity(e));
        cr_assert(gm.getComponent<Health>(e).hp == 1);
        cr_assert(!gm.hasComponent<Position>(e));
    }

    Test(tricky, staleHandleCannotRemoveEveryComponent,
        .description = "[generation] a stale handle cannot removeComponents")
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
        cr_assert(gm.getComponent<Health>(fresh).hp == 9);
        cr_assert(gm.hasComponent<Position>(fresh));
    }

    Test(tricky, staleHandleRemovesEverythingDuringUpdate,
        .description = "[generation] a stale removeComponents during update")
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
        cr_assert(gm.getComponent<Health>(player).hp == 3);
        cr_assert(gm.hasComponent<Player>(player));
    }

    Test(tricky, removeEverythingThenDestroyInSameFrame,
        .description =
            "[frame] destroy then removeComponents in the same frame")
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
        cr_assert_throw(gm.update(), RType::NoSuchEntity);
        cr_assert(!gm.hasEntity(target));
        Entity fresh = gm.createEntity();
        cr_assert(indexOf(fresh) == indexOf(target));
        cr_assert(!gm.hasComponent<Health>(fresh));
    }

} // namespace
