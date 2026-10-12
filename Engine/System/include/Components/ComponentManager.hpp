/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentManager
*/

#ifndef COMPONENTMANAGER_HPP_
#define COMPONENTMANAGER_HPP_

#include <memory>
#include <typeindex>
#include <unordered_map>
#include "ComponentArray.hpp"
#include "Entity.hpp"

namespace RType {

    /**
     * @brief Maps each component type to its ComponentArray.
     */
    using ComponentList =
        std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>>;

    /**
     * @brief Owns and gives access to every ComponentArray in use.
     */
    class ComponentManager {
    public:
        /**
         * @brief Constructs and stores a component of the given type for
         * an entity.
         * @tparam Component The component type to add.
         * @tparam Args Constructor argument types forwarded to Component.
         * @param entity The entity to attach the component to.
         * @param args Arguments forwarded to Component's constructor.
         * @return Reference to the newly created component.
         */
        template<typename Component, typename... Args>
        Component &addComponent(Entity entity, Args &&...args);

        /**
         * @brief Removes a component of the given type from an entity.
         * @tparam Component The component type to remove.
         * @param entity The entity to remove the component from.
         */
        template<typename Component>
        void removeComponent(Entity entity);

        /**
         * @brief Removes every component of every type for an entity.
         * @param entity The entity whose components should all be
         * removed.
         */
        void removeComponent(Entity entity);

        /**
         * @brief Retrieves a component of the given type for an entity.
         * @tparam Component The component type requested.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         */
        template<typename Component>
        Component &getComponent(Entity entity);

        /**
         * @brief Checks whether an entity has a component of the given
         * type.
         * @tparam Component The component type to check.
         * @param entity The entity to check.
         * @return true if the entity has that component, false
         * otherwise.
         */
        template<typename Component>
        [[nodiscard]] bool hasComponent(Entity entity) const;

        /**
         * @brief Retrieves the ComponentArray storing every instance of
         * the given type.
         * @tparam Component The component type.
         * @return Reference to the corresponding ComponentArray.
         */
        template<typename Component>
        ComponentArray<Component> &getComponentArray();

    private:
        ComponentList _components;
    };

} // namespace RType

#endif /* !COMPONENTMANAGER_HPP_ */
