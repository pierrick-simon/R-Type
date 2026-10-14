/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity
*/

#include <algorithm>

#include "ComponentManager.hpp"
#include "Entity.hpp"

namespace RType {

    EntityManager::EntityManager()
    {
    }

    Entity EntityManager::createEntity()
    {
        _entities.push_back(_nextID);
        _nextID += 1;
        return _nextID - 1;
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        _entities.erase(std::find(_entities.begin(), _entities.end(), entity));
    }

    bool EntityManager::hasEntity(Entity entity)
    {
        return std::count(_entities.begin(), _entities.end(), entity) > 0;
    }

} // namespace RType
