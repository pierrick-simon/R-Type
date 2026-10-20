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
            auto prev = _toRecycle.front();
            Entity e(prev.index(), prev.generation() + 1);
            _entities[e.index()] = {true, e.generation()};
            _toRecycle.pop();
            return e;
        }
        _entities.push_back({true, 0});
        return Entity(_entities.size() - 1);
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        if (!hasEntity(entity))
            throw NoSuchEntity();
        _entities[entity.index()] = {false};
        _toRecycle.push(entity);
    }

    bool EntityManager::hasEntity(Entity entity) const
    {
        return (_entities.size() > entity.index() &&
            _entities[entity.index()].isActive &&
            _entities[entity.index()].generation == entity.generation());
    }

    Entity::Entity(std::size_t index, std::size_t generation) :
        _index(index), _generation(generation)
    {
    }

    bool Entity::operator==(const Entity &entity) const
    {
        return (this->index() == entity.index() &&
            this->generation() == entity.generation());
    }

    std::size_t Entity::index() const
    {
        return _index;
    }

    std::size_t Entity::generation() const
    {
        return _generation;
    }

} // namespace RType
