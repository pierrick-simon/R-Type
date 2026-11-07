/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_duplicates
*/

#include <cstddef>
#include <string>
#include <type_traits>

#include <criterion/criterion.h>

#include "GameManager.hpp"

// Expected behaviour: adding a component to an entity that already has one
// of the same type throws AlreadyExistingComponent and keeps the first one.
// Outside update(), it throws right away. During update(), the add is
// deferred, so update() throws once the frame has been applied.

namespace {

    struct Bullet {};

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

    Test(duplicates, throwsAndKeepsTheFirstOne,
        .description = "[duplicate] throws and keeps the first one")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 2.f);
        cr_assert_throw(gm.addComponent<Position>(e, 9.f, 9.f),
            RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Position>(e).x == 1.f);
        cr_assert(gm.getComponent<Position>(e).y == 2.f);
        cr_assert(gm.getArray<Position>().nbComponent() == 1);
    }

    Test(duplicates, throwsForAnEmptyTag,
        .description = "[duplicate] throws for an empty tag")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Bullet>(e);
        cr_assert_throw(
            gm.addComponent<Bullet>(e), RType::AlreadyExistingComponent);
        cr_assert(gm.getArray<Bullet>().nbComponent() == 1);
    }

    Test(duplicates, throwsEveryTime,
        .description = "[duplicate] throws every time")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        for (int i = 0; i < 5; i++)
            cr_assert_throw(
                gm.addComponent<Health>(e, i), RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Health>(e).hp == 1);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(duplicates, isAnEngineException,
        .description = "[duplicate] is an EngineException")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Health>(e, 1);
        cr_assert_throw(gm.addComponent<Health>(e, 2), RType::EngineException);
        try {
            gm.addComponent<Health>(e, 3);
        } catch (const RType::AlreadyExistingComponent &error) {
            cr_assert(
                std::string(error.what()).find("Already") != std::string::npos);
        }
    }

    Test(duplicates, sameTypeOnAnotherEntityDoesNotThrow,
        .description =
            "[duplicate] the same type on another entity does not throw")
    {
        GameManager gm;
        Entity a = gm.createEntity();
        Entity b = gm.createEntity();

        gm.addComponent<Health>(a, 1);
        cr_assert_no_throw(gm.addComponent<Health>(b, 2), std::exception);
        cr_assert(gm.getComponent<Health>(a).hp == 1);
        cr_assert(gm.getComponent<Health>(b).hp == 2);
    }

    Test(duplicates, anotherTypeOnTheSameEntityDoesNotThrow,
        .description =
            "[duplicate] another type on the same entity does not throw")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.addComponent<Position>(e, 1.f, 1.f);
        cr_assert_no_throw(
            gm.addComponent<Velocity>(e, 2.f, 2.f), std::exception);
        cr_assert_no_throw(gm.addComponent<Health>(e, 3), std::exception);
    }

    Test(duplicates, recycledEntityDoesNotThrow,
        .description = "[duplicate] a recycled entity does not throw")
    {
        GameManager gm;
        Entity old = gm.createEntity();

        gm.addComponent<Health>(old, 1);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        cr_assert(indexOf(reused) == indexOf(old));
        cr_assert_no_throw(gm.addComponent<Health>(reused, 2), std::exception);
        cr_assert(gm.getComponent<Health>(reused).hp == 2);
    }

    Test(duplicates, staleHandleThrowsNoSuchEntity,
        .description = "[duplicate] a stale handle throws NoSuchEntity")
    {
        GameManager gm;
        Entity old = gm.createEntity();

        gm.addComponent<Health>(old, 1);
        gm.destroyEntity(old);
        Entity reused = gm.createEntity();
        gm.addComponent<Health>(reused, 2);
        cr_assert_throw(gm.addComponent<Health>(old, 3), RType::NoSuchEntity);
        cr_assert(gm.getComponent<Health>(reused).hp == 2);
    }

    Test(duplicates, refusedComponentIsDestroyed,
        .description = "[duplicate] the refused component is destroyed")
    {
        Tracked::alive = 0;
        {
            GameManager gm;
            Entity e = gm.createEntity();

            gm.addComponent<Tracked>(e, 1);
            cr_assert_throw(gm.addComponent<Tracked>(e, 2),
                RType::AlreadyExistingComponent);
            cr_assert(Tracked::alive == 1);
            cr_assert(gm.getComponent<Tracked>(e).value == 1);
        }
        cr_assert(Tracked::alive == 0);
    }

    Test(duplicates, insideASystemThrowsAtTheEndOfUpdate,
        .description = "[duplicate] inside a system, update() throws")
    {
        GameManager gm;
        Entity e = gm.createEntity();
        bool reached = false;

        gm.addComponent<Health>(e, 5);
        gm.createSystem([&gm, &reached, e] {
            cr_assert_no_throw(gm.addComponent<Health>(e, 99), std::exception);
            reached = true;
        });
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert(reached);
        cr_assert(gm.getComponent<Health>(e).hp == 5);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(duplicates, insideAPerEntitySystem,
        .description = "[duplicate] inside a per-entity system")
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
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        int sum = 0;
        gm.view<Health>().each([&sum](Entity, Health &h) { sum += h.hp; });
        cr_assert(sum == 0 + 1 + 2 + 3);
        cr_assert(gm.getArray<Health>().nbComponent() == 4);
    }

    Test(duplicates, twoAddsInTheSameFrameKeepTheFirst,
        .description = "[duplicate] two adds in the same frame keep the first")
    {
        GameManager gm;
        Entity e = gm.createEntity();

        gm.createSystem([&gm, e] {
            gm.addComponent<Health>(e, 1);
            gm.addComponent<Health>(e, 2);
        });
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Health>(e).hp == 1);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(duplicates, twoSystemsCheckingHasComponentStillCollide,
        .description =
            "[duplicate] two systems checking hasComponent still collide")
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
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Health>(e).hp == 1);
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(gm.getComponent<Health>(e).hp == 1);
    }

    Test(duplicates, otherCommandsOfTheFrameAreApplied,
        .description =
            "[duplicate] the other commands of the frame are applied")
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
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert(gm.getComponent<Health>(e).hp == 1);
        cr_assert(gm.getComponent<Position>(other).x == 3.f);
        cr_assert(!gm.hasEntity(doomed));
    }

    Test(duplicates, nextFramesStillWork,
        .description = "[duplicate] the next frames still work")
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
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert_no_throw(
            gm.addComponent<Position>(e, 1.f, 1.f), std::exception);
        cr_assert(frames == 2);
        cr_assert(gm.hasComponent<Position>(e));
    }

    Test(duplicates, removeThenAddInTheSameFrameDoesNotThrow,
        .description =
            "[duplicate] remove then add in the same frame does not throw")
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
        cr_assert_no_throw(gm.update(), std::exception);
        cr_assert(gm.getComponent<Health>(e).hp == 2);
        cr_assert(gm.getArray<Health>().nbComponent() == 1);
    }

    Test(duplicates, addThenRemoveInTheSameFrame,
        .description = "[duplicate] add then remove in the same frame")
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
        cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
        cr_assert(!gm.hasComponent<Health>(e));
    }

    Test(duplicates, addAfterDestroyInTheSameFrame,
        .description = "[duplicate] add after destroy in the same frame")
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
        cr_assert_throw(gm.update(), RType::NoSuchEntity);
        cr_assert(!gm.hasEntity(e));
    }

    Test(duplicates, refusedComponentIsDestroyedDuringUpdate,
        .description =
            "[duplicate] the refused component is destroyed during update")
    {
        Tracked::alive = 0;
        {
            GameManager gm;
            Entity e = gm.createEntity();

            gm.addComponent<Tracked>(e, 1);
            gm.createSystem([&gm, e] { gm.addComponent<Tracked>(e, 2); });
            cr_assert_throw(gm.update(), RType::AlreadyExistingComponent);
            cr_assert(Tracked::alive == 1);
            cr_assert(gm.getComponent<Tracked>(e).value == 1);
        }
        cr_assert(Tracked::alive == 0);
    }

} // namespace
