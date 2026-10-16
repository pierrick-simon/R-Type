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
        if (_updating)
            return;
        _updating = true;
        try {
            _sm.process(_cm);
        } catch (...) {
            _updating = false;
            try {
                flush();
            } catch (...) {
            };
            throw;
        }
        _updating = false;
        flush();
    }

    Entity GameManager::createEntity()
    {
        return _em.createEntity();
    }

    void GameManager::destroyEntity(Entity entity)
    {
        _cq.emplace_back([this, entity]() {
            if (!hasEntity(entity))
                return;
            _cm.removeComponents(entity);
            _em.destroyEntity(entity);
        });
        tryFlush();
    }

    bool GameManager::hasEntity(Entity entity) const
    {
        return _em.hasEntity(entity);
    }

    void GameManager::removeComponents(Entity entity)
    {
        _cq.emplace_back([this, entity]() {
            if (!hasEntity(entity))
                throw NoSuchEntity();
            _cm.removeComponents(entity);
        });
        tryFlush();
    }

    void GameManager::destroySystem(SystemId id)
    {
        _cq.emplace_back([this, id]() {
            if (!hasSystem(id))
                return;
            _sm.destroySystem(id);
        });
        tryFlush();
    }

    bool GameManager::hasSystem(SystemId id) const
    {
        return _sm.hasSystem(id);
    }

    void GameManager::flush()
    {
        std::exception_ptr error = nullptr;

        for (auto &command : _cq) {
            try {
                command();
            } catch (...) {
                if (!error)
                    error = std::current_exception();
            }
        }
        _cq.clear();
        if (error)
            std::rethrow_exception(error);
    }

    void GameManager::tryFlush()
    {
        if (!_updating)
            flush();
    }

} // namespace RType
