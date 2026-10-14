/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#include "ComponentManager.hpp"

namespace RType {

    ComponentManager::ComponentManager() {};

    void ComponentManager::removeComponents(Entity entity)
    {
        for (auto &[_, componentArray]: _components)
            if (componentArray->hasComponent(entity))
                componentArray->removeEntity(entity);
    }

}
