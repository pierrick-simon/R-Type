/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Duplicates
*/

#include <string>

#include "Components.hpp"
#include "EntityUtils.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

// Expected behaviour: adding a component to an entity that already has one
// of the same type throws AlreadyExistingComponent and keeps the first one.
// Outside update(), it throws right away. During update(), the add is
// deferred, so update() throws once the frame has been applied.

namespace {

    using RType::Entity;
    using RType::GameManager;
    using namespace Poc;

    void throwsAndKeepsTheFirstOne()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 2.f);
        POC_CHECK_THROWS(gm.addComponent<Position>(e, 9.f, 9.f),
            RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Position>(e).x == 1.f);
        POC_CHECK(gm.getComponent<Position>(e).y == 2.f);
        POC_CHECK(gm.getArray<Position>().nbComponent() == 1);
    }

    void throwsForAnEmptyTag()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Bullet>(e);
        POC_CHECK_THROWS(
            gm.addComponent<Bullet>(e), RType::AlreadyExistingComponent);
        POC_CHECK(gm.getArray<Bullet>().nbComponent() == 1);
    }

    void throwsEveryTime()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        for (int i = 0; i < 5; i++)
            POC_CHECK_THROWS(gm.addComponent<Health>(e, i),
                RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void isAnEngineException()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        POC_CHECK_THROWS(
            gm.addComponent<Health>(e, 2), RType::EngineException);
        try {
            gm.addComponent<Health>(e, 3);
        } catch (const RType::AlreadyExistingComponent &error) {
            POC_CHECK(std::string(error.what()).find("Already") !=
                std::string::npos);
        }
    }

    void sameTypeOnAnotherEntityDoesNotThrow()
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        POC_CHECK_NOTHROW(gm.addComponent<Health>(b, 2));
        POC_CHECK(gm.getComponent<Health>(a).hp == 1);
        POC_CHECK(gm.getComponent<Health>(b).hp == 2);
    }

    void anotherTypeOnTheSameEntityDoesNotThrow()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        POC_CHECK_NOTHROW(gm.addComponent<Velocity>(e, 2.f, 2.f));
        POC_CHECK_NOTHROW(gm.addComponent<Health>(e, 3));
    }

    void recycledEntityDoesNotThrow()
    {
        GameManager gm;
        Entity old = gm.createEntity();

        gm.addComponent<Health>(old, 1);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        POC_CHECK(indexOf(reused) == indexOf(old));
        POC_CHECK_NOTHROW(gm.addComponent<Health>(reused, 2));
        POC_CHECK(gm.getComponent<Health>(reused).hp == 2);
    }

    void staleHandleThrowsNoSuchEntity()
    {
        GameManager gm;
        Entity old = gm.createEntity();

        gm.addComponent<Health>(old, 1);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        gm.addComponent<Health>(reused, 2);
        POC_CHECK_THROWS(
            gm.addComponent<Health>(old, 3), RType::NoSuchEntity);
        POC_CHECK(gm.getComponent<Health>(reused).hp == 2);
    }

    void refusedComponentIsDestroyed()
    {
        Tracked::alive = 0;
        {
            GameManager gm;
            Entity e = gm.createEntity();

            gm.addComponent<Tracked>(e, 1);
            POC_CHECK_THROWS(gm.addComponent<Tracked>(e, 2),
                RType::AlreadyExistingComponent);
            POC_CHECK(Tracked::alive == 1);
            POC_CHECK(gm.getComponent<Tracked>(e).value == 1);
        }
        POC_CHECK(Tracked::alive == 0);
    }

    void insideASystemThrowsAtTheEndOfUpdate()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool reached = false;

        gm.addComponent<Health>(e, 5);
        gm.createSystem([&gm, &reached, e] {
            POC_CHECK_NOTHROW(gm.addComponent<Health>(e, 99));
            reached = true;
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK(reached);
        POC_CHECK(gm.getComponent<Health>(e).hp == 5);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void insideAPerEntitySystem()
    {
        GameManager gm;

        for (int i = 0; i < 4; i++)
            gm.addComponent<Health>(gm.createEntity(), i);
        gm.createSystem<Health>([](GameManager &g, Health &h) {
            g.view<Health>().each([&g, &h](Entity e, Health &other) {
                if (&other == &h)
                    g.addComponent<Health>(e, -1);
            });
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        int sum = 0;
        gm.view<Health>().each([&sum](Entity, Health &h) { sum += h.hp; });
        POC_CHECK(sum == 0 + 1 + 2 + 3);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 4);
    }

    void twoAddsInTheSameFrameKeepTheFirst()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.createSystem([&gm, e] {
            gm.addComponent<Health>(e, 1);
            gm.addComponent<Health>(e, 2);
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void twoSystemsCheckingHasComponentStillCollide()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.createSystem([&gm, e] {
            if (!gm.hasComponent<Health>(e))
                gm.addComponent<Health>(e, 1);
        });
        gm.createSystem([&gm, e] {
            if (!gm.hasComponent<Health>(e))
                gm.addComponent<Health>(e, 2);
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
    }

    void otherCommandsOfTheFrameAreApplied()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        Entity other = gm.createEntity();
        Entity doomed = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.createSystem([&gm, e, other, doomed] {
            gm.addComponent<Health>(e, 2);
            gm.addComponent<Position>(other, 3.f, 3.f);
            gm.destroyEntity(doomed);
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK(gm.getComponent<Health>(e).hp == 1);
        POC_CHECK(gm.getComponent<Position>(other).x == 3.f);
        POC_CHECK(!gm.hasEntity(doomed));
    }

    void nextFramesStillWork()
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool once = true;
        int frames = 0;

        gm.addComponent<Health>(e, 1);
        gm.createSystem([&gm, &once, &frames, e] {
            frames++;
            if (once)
                gm.addComponent<Health>(e, 2);
            once = false;
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK_NOTHROW(gm.addComponent<Position>(e, 1.f, 1.f));
        POC_CHECK(frames == 2);
        POC_CHECK(gm.hasComponent<Position>(e));
    }

    void removeThenAddInTheSameFrameDoesNotThrow()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.createSystem([&gm, e] {
            if (gm.getComponent<Health>(e).hp != 1)
                return;
            gm.removeComponent<Health>(e);
            gm.addComponent<Health>(e, 2);
        });
        POC_CHECK_NOTHROW(gm.update());
        POC_CHECK(gm.getComponent<Health>(e).hp == 2);
        POC_CHECK(gm.getArray<Health>().nbComponent() == 1);
    }

    void addThenRemoveInTheSameFrame()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.createSystem([&gm, e] {
            if (!gm.hasComponent<Health>(e))
                return;
            gm.addComponent<Health>(e, 2);
            gm.removeComponent<Health>(e);
        });
        POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
        POC_CHECK(!gm.hasComponent<Health>(e));
    }

    void addAfterDestroyInTheSameFrame()
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        gm.createSystem([&gm, e] {
            if (!gm.hasEntity(e))
                return;
            gm.destroyEntity(e);
            gm.addComponent<Health>(e, 2);
        });
        POC_CHECK_THROWS(gm.update(), RType::NoSuchEntity);
        POC_CHECK(!gm.hasEntity(e));
    }

    void refusedComponentIsDestroyedDuringUpdate()
    {
        Tracked::alive = 0;
        {
            GameManager gm;
            Entity e = gm.createEntity();

            gm.addComponent<Tracked>(e, 1);
            gm.createSystem([&gm, e] { gm.addComponent<Tracked>(e, 2); });
            POC_CHECK_THROWS(gm.update(), RType::AlreadyExistingComponent);
            POC_CHECK(Tracked::alive == 1);
            POC_CHECK(gm.getComponent<Tracked>(e).value == 1);
        }
        POC_CHECK(Tracked::alive == 0);
    }

} // namespace

namespace Poc {

    TestList duplicateTests()
    {
        return {
            {"[duplicate] throws and keeps the first one",
                throwsAndKeepsTheFirstOne},
            {"[duplicate] throws for an empty tag", throwsForAnEmptyTag},
            {"[duplicate] throws every time", throwsEveryTime},
            {"[duplicate] is an EngineException", isAnEngineException},
            {"[duplicate] the same type on another entity does not throw",
                sameTypeOnAnotherEntityDoesNotThrow},
            {"[duplicate] another type on the same entity does not throw",
                anotherTypeOnTheSameEntityDoesNotThrow},
            {"[duplicate] a recycled entity does not throw",
                recycledEntityDoesNotThrow},
            {"[duplicate] a stale handle throws NoSuchEntity",
                staleHandleThrowsNoSuchEntity},
            {"[duplicate] the refused component is destroyed",
                refusedComponentIsDestroyed},
            {"[duplicate] inside a system, update() throws",
                insideASystemThrowsAtTheEndOfUpdate},
            {"[duplicate] inside a per-entity system",
                insideAPerEntitySystem},
            {"[duplicate] two adds in the same frame keep the first",
                twoAddsInTheSameFrameKeepTheFirst},
            {"[duplicate] two systems checking hasComponent still collide",
                twoSystemsCheckingHasComponentStillCollide},
            {"[duplicate] the other commands of the frame are applied",
                otherCommandsOfTheFrameAreApplied},
            {"[duplicate] the next frames still work", nextFramesStillWork},
            {"[duplicate] remove then add in the same frame does not throw",
                removeThenAddInTheSameFrameDoesNotThrow},
            {"[duplicate] add then remove in the same frame",
                addThenRemoveInTheSameFrame},
            {"[duplicate] add after destroy in the same frame",
                addAfterDestroyInTheSameFrame},
            {"[duplicate] the refused component is destroyed during update",
                refusedComponentIsDestroyedDuringUpdate},
        };
    }

} // namespace Poc
