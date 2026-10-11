/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#ifndef COMPONENTMANAGER_HPP_
#define COMPONENTMANAGER_HPP_

#include "ComponentArray.hpp"
#include "Entity.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace RType {

    using ComponentList = std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>>;

    class ComponentManager {
        public:
            template<typename Component, typename... Args>
            Component &addComponent(Entity entity, Args &&...args);

            template<typename Component>
            void removeComponent(Entity entity);

            void removeComponent(Entity entity);

            template<typename Component>
            Component &getComponent(Entity entity);

            template<typename Component>
            bool hasComponent(Entity entity) const;

            template<typename Component>
            ComponentArray<Component> &getComponentArray();

        private:
            ComponentList _components;
    };

}

#endif /* !COMPONENTMANAGER_HPP_ */
