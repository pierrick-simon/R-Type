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

    struct EnemyHooks {
        std::function<void(GameManager &, Entity, const EnemyDef &)>
            onEnemySpawn;
        std::function<void(GameManager &, Entity)> onBulletSpawn;
        std::function<void(GameManager &, float x, float y)> onEnemyDeath;
    };

    struct SpawnEvent {
        float time = 0.f;
        EnemyType type = EnemyType::Scout;
        float y = 0.f;
    };

    struct EnemyDirector {
        std::vector<SpawnEvent> timeline;
        std::size_t next = 0;
        float elapsed = 0.f;
        float restartDelay = 4.f;
        int score = 0;

        void addWave(
            EnemyType type, float y, int count, float interval, float start);
    };

    Entity spawnEnemy(GameManager &gm, EnemyType type, float x, float y,
        const EnemyHooks &hooks = {});

    Entity fireEnemyBullet(GameManager &gm, float x, float y, float vx,
        float vy, int damage, const EnemyHooks &hooks = {});

    void resetEnemies(GameManager &gm);

    Entity createEnemyDirector(GameManager &gm);

} // namespace RType

#endif
