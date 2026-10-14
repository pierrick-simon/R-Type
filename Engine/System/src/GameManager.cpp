/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManager
*/

#include "GameManager.hpp"

namespace RType {

    GameManager::GameManager() :
        _cm(), _em(), _sm()
    {
    }

    void GameManager::update()
    {
        _sm.process(_cm);
    }

    Entity GameManager::createEntity()
    {
        return _em.createEntity();
    }

    void GameManager::destroyEntity(Entity entity)
    {
        _cm.removeComponents(entity);
        _em.destroyEntity(entity);
    }

    bool GameManager::hasEntity(Entity entity)
    {
        return _em.hasEntity(entity);
    }

    void GameManager::removeComponents(Entity entity)
    {
        _cm.removeComponents(entity);
    }

    void GameManager::destroySystem(SystemList::iterator system)
    {
        _sm.destroySystem(system);
    }

} // namespace RType
