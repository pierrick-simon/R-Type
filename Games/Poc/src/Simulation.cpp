/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Simulation
*/

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

#include "Components.hpp"
#include "GameManager.hpp"
#include "Test.hpp"

namespace {

    using RType::Entity;
    using RType::GameManager;
    using namespace Poc;

    constexpr float SCREEN_WIDTH = 200.f;
    constexpr float SCREEN_HEIGHT = 100.f;
    constexpr int TICKS = 600;

    class World {
    public:
        World()
        {
            _player = spawn(10.f, 50.f, 0.f, 1.f);
            _gm.addComponent<Health>(_player, 3);
            _gm.addComponent<Name>(_player, "player");
            _gm.addComponent<Player>(_player);
        }

        void tick(int tick)
        {
            if (tick % 7 == 0)
                spawnEnemy(tick);
            if (tick % 4 == 0 && _gm.hasComponent<Player>(_player))
                fire();
            move();
            bouncePlayer();
            collide();
            cleanup();
            checkInvariants();
        }

        void shutdown()
        {
            for (Entity e : _entities)
                _gm.destroyEntity(e);
            _entities.clear();
            POC_CHECK(_gm.getArray<Position>().nbComponent() == 0);
            POC_CHECK(_gm.getArray<Velocity>().nbComponent() == 0);
            POC_CHECK(_gm.getArray<Health>().nbComponent() == 0);
            POC_CHECK(_gm.getArray<Damage>().nbComponent() == 0);
            POC_CHECK(_gm.getArray<Enemy>().nbComponent() == 0);
            POC_CHECK(_gm.getArray<Bullet>().nbComponent() == 0);
        }

        int kills = 0;
        int hitsTaken = 0;
        int bulletsFired = 0;
        int playerDiedAt = -1;
        std::size_t maxAlive = 0;

    private:
        Entity spawn(float x, float y, float vx, float vy)
        {
            Entity e = _gm.createEntity();

            _gm.addComponent<Position>(e, x, y);
            _gm.addComponent<Velocity>(e, vx, vy);
            _entities.push_back(e);
            maxAlive = std::max(maxAlive, _entities.size());
            return e;
        }

        void spawnEnemy(int tick)
        {
            auto y = static_cast<float>((tick * 13) % 100);
            Entity e = spawn(SCREEN_WIDTH, y, -1.5f, 0.f);

            _gm.addComponent<Health>(e, 2);
            _gm.addComponent<Enemy>(e);
        }

        void fire()
        {
            const Position &pos = _gm.getComponent<Position>(_player);
            Entity e = spawn(pos.x, pos.y, 4.f, 0.f);

            _gm.addComponent<Damage>(e, 1);
            _gm.addComponent<Bullet>(e);
            bulletsFired++;
        }

        void move()
        {
            for (Entity e : _entities) {
                if (!_gm.hasComponent<Position>(e) ||
                    !_gm.hasComponent<Velocity>(e))
                    continue;
                auto &pos = _gm.getComponent<Position>(e);
                const Velocity &vel = _gm.getComponent<Velocity>(e);
                pos.x += vel.x;
                pos.y += vel.y;
            }
        }

        void bouncePlayer()
        {
            if (!_gm.hasComponent<Player>(_player))
                return;
            const Position &pos = _gm.getComponent<Position>(_player);
            auto &vel = _gm.getComponent<Velocity>(_player);
            if (pos.y <= 0.f || pos.y >= SCREEN_HEIGHT)
                vel.y = -vel.y;
        }

        void collide()
        {
            for (Entity bullet : _entities) {
                if (!_gm.hasComponent<Bullet>(bullet) || _dead.contains(bullet))
                    continue;
                const Position &bpos = _gm.getComponent<Position>(bullet);
                if (bpos.x > SCREEN_WIDTH) {
                    _dead.insert(bullet);
                    continue;
                }
                for (Entity enemy : _entities) {
                    if (!_gm.hasComponent<Enemy>(enemy))
                        continue;
                    const Position &epos = _gm.getComponent<Position>(enemy);
                    if (std::abs(bpos.x - epos.x) > 3.f ||
                        std::abs(bpos.y - epos.y) > 3.f)
                        continue;
                    auto &hp = _gm.getComponent<Health>(enemy);
                    hp.hp -= _gm.getComponent<Damage>(bullet).value;
                    if (hp.hp <= 0 && !_dead.contains(enemy)) {
                        _dead.insert(enemy);
                        kills++;
                    }
                    _dead.insert(bullet);
                    break;
                }
            }
            for (Entity enemy : _entities) {
                if (!_gm.hasComponent<Enemy>(enemy) ||
                    _gm.getComponent<Position>(enemy).x >= 0.f)
                    continue;
                _dead.insert(enemy);
                if (!_gm.hasComponent<Player>(_player))
                    continue;
                hitsTaken++;
                if (--_gm.getComponent<Health>(_player).hp <= 0)
                    _dead.insert(_player);
            }
        }

        void cleanup()
        {
            if (_dead.contains(_player) && playerDiedAt < 0)
                playerDiedAt = _tick;
            for (Entity e : _dead)
                _gm.destroyEntity(e);
            std::erase_if(
                _entities, [this](Entity e) { return _dead.contains(e); });
            _dead.clear();
            _tick++;
        }

        void checkInvariants()
        {
            std::size_t enemies = 0;
            std::size_t bullets = 0;

            for (Entity e : _entities) {
                POC_CHECK(_gm.hasComponent<Position>(e));
                POC_CHECK(_gm.hasComponent<Velocity>(e));
                enemies += _gm.hasComponent<Enemy>(e) ? 1 : 0;
                bullets += _gm.hasComponent<Bullet>(e) ? 1 : 0;
                if (_gm.hasComponent<Bullet>(e))
                    POC_CHECK(_gm.hasComponent<Damage>(e));
                if (_gm.hasComponent<Enemy>(e))
                    POC_CHECK(_gm.getComponent<Health>(e).hp > 0);
            }
            POC_CHECK(
                _gm.getArray<Position>().nbComponent() == _entities.size());
            POC_CHECK(
                _gm.getArray<Velocity>().nbComponent() == _entities.size());
            POC_CHECK(_gm.getArray<Enemy>().nbComponent() == enemies);
            POC_CHECK(_gm.getArray<Bullet>().nbComponent() == bullets);
            POC_CHECK(_gm.getArray<Damage>().nbComponent() == bullets);
        }

        GameManager _gm;
        std::vector<Entity> _entities;
        std::set<Entity> _dead;
        Entity _player;
        int _tick = 0;
    };

    void miniShootEmUp()
    {
        World world;

        for (int tick = 0; tick < TICKS; tick++)
            world.tick(tick);
        std::cout << "         " << TICKS << " ticks, " << world.bulletsFired
                  << " bullets, " << world.kills << " kills, "
                  << world.hitsTaken << " hits taken, player died at tick "
                  << world.playerDiedAt << ", peak " << world.maxAlive
                  << " entities" << std::endl;
        POC_CHECK(world.kills > 0);
        POC_CHECK(world.hitsTaken > 0);
        world.shutdown();
    }

} // namespace

namespace Poc {

    TestList simulationTests()
    {
        return {
            {"mini shoot'em up simulation", miniShootEmUp},
        };
    }

} // namespace Poc
