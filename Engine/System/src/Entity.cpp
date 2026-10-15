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
        _entities.push_back(true);
        return _entities.size() - 1;
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        if (!hasEntity(entity))
            throw NoSuchEntity();
        _entities[entity] = false;
    }

    bool EntityManager::hasEntity(Entity entity) const
    {
        return (_entities.size() > entity && _entities[entity]);
    }

} // namespace RType
