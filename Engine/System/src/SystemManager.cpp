/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#include "SystemManager.hpp"

namespace RType {

    SystemManager::SystemManager()
    {
    }

    void SystemManager::process(ComponentManager &cm)
    {
        for (auto &system : _systems)
            system->update(cm);
    }

    void SystemManager::destroySystem(SystemList::iterator system)
    {
        _systems.erase(system);
    }

} // namespace RType
