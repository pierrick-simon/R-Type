/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#include <algorithm>

#include "SystemManager.hpp"

namespace RType {

    void SystemManager::process(ComponentManager &cm)
    {
        for (auto &entry : _systems)
            entry.system->update(cm);
    }

    void SystemManager::destroySystem(SystemId id)
    {
        auto it = std::ranges::find(_systems, id, &SystemEntry::id);

        if (it == _systems.end())
            throw NoSuchSystem();
        _systems.erase(it);
    }

    bool SystemManager::hasSystem(SystemId id) const
    {
        return std::ranges::find(_systems, id, &SystemEntry::id) !=
            _systems.end();
    }

} // namespace RType
