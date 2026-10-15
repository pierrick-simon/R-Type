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

    Entity EntityManager::createEntity()
    {
        if (!_toRecycle.empty()) {
            auto e = _toRecycle.front();
            _entities[e] = true;
            _toRecycle.pop();
            return e;
        }
        _entities.push_back(true);
        return _entities.size() - 1;
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        if (!hasEntity(entity))
            throw NoSuchEntity();
        _entities[entity] = false;
        _toRecycle.push(entity);
    }

    bool EntityManager::hasEntity(Entity entity) const
    {
        return (_entities.size() > entity && _entities[entity]);
    }

} // namespace RType
