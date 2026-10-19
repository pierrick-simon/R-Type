/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemyManager.hpp
*/

#ifndef ENEMYMANAGER_HPP_
    #define ENEMYMANAGER_HPP_
    #include <cstddef>
    #include <functional>
    #include <vector>
    #include "EnemyDefs.hpp"
    #include "GameManager.hpp"

namespace RType {

    /**
     * @brief Optional callbacks used to attach what is NOT game logic
     * (textures, sprite sheets, sounds...) to the entities created by the
     * enemy code. Left empty, the enemies are pure logic: no window needed.
     *
     * The callbacks run while a frame is updating, so components added there
     * are only available at the end of the frame (see GameManager).
     */
    struct EnemyHooks {
        std::function<void(GameManager &, Entity, const EnemyDef &)>
            onEnemySpawn;
        std::function<void(GameManager &, Entity)> onBulletSpawn;
        /** x / y: centre of the destroyed enemy. */
        std::function<void(GameManager &, float x, float y)> onEnemyDeath;
    };

    struct SpawnEvent {
        float time = 0.f; // seconds since the start of the timeline
        EnemyType type = EnemyType::Scout;
        float y = 0.f;
    };

    /**
     * @brief Singleton component: the level script (who spawns when) and the
     * score. Enemy systems that need the GameManager (spawn, cleanup,
     * collisions) are attached to it, as ECS systems only receive components.
     */
    struct EnemyDirector {
        std::vector<SpawnEvent> timeline;
        std::size_t next = 0;
        float elapsed = 0.f;
        float restartDelay = 4.f; // seconds after the last spawn; < 0: no loop
        int score = 0;

        /** Spawn @p count enemies, one every @p interval seconds from @p start.
         */
        void addWave(
            EnemyType type, float y, int count, float interval, float start);
    };

    /** Creates an enemy just right of the screen at height @p y. */
    Entity spawnEnemy(GameManager &gm, EnemyType type, float x, float y,
        const EnemyHooks &hooks = {});

    /** Creates a bullet that hurts EnemyTarget entities. */
    Entity fireEnemyBullet(GameManager &gm, float x, float y, float vx,
        float vy, int damage, const EnemyHooks &hooks = {});

    /** Creates the director entity (EnemyDirector + Clock) with a demo level.
     */
    Entity createEnemyDirector(GameManager &gm);

} // namespace RType

#endif
