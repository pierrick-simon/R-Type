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
#include <utility>

#include "ComponentArray.hpp"
#include "Entity.hpp"
#include "Exceptions.hpp"

namespace RType {

    class NoSuchComponent;

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
         * @brief Constructs an empty ComponentManager.
         */
        explicit ComponentManager();

        /**
         * @brief Copy construction is disabled.
         */
        ComponentManager(const ComponentManager &) = delete;

        /**
         * @brief Copy assignment is disabled.
         */
        ComponentManager &operator=(const ComponentManager &) = delete;

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
        Component &addComponent(Entity entity, Args &&...args)
        {
            auto index = std::type_index(typeid(Component));
            if (isComponent<Component>()) {
                auto &array = static_cast<ComponentArray<Component> &>(*_components.at(index));
                return array.addEntity(entity, std::forward<Args>(args)...);
            }
            auto array = std::make_unique<ComponentArray<Component>>();
            auto &tmp = array->addEntity(entity, std::forward<Args>(args)...);
            _components.emplace(index, std::move(array));
            return tmp;
        }

        /**
         * @brief Removes a component of the given type from an entity.
         * @tparam Component The component type to remove.
         * @param entity The entity to remove the component from.
         */
        template<typename Component>
        void removeComponent(Entity entity)
        {
            auto index = std::type_index(typeid(Component));
            if (!isComponent<Component>())
                throw NoSuchComponent("Could not remove link between entity and component.");
            _components.at(index)->removeEntity(entity);
        }

        /**
         * @brief Removes every component of every type for an entity.
         * @param entity The entity whose components should all be
         * removed.
         */
        void removeComponents(Entity entity);

        /**
         * @brief Retrieves a component of the given type for an entity.
         * @tparam Component The component type requested.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         */
        template<typename Component>
        [[nodiscard]] Component &getComponent(Entity entity)
        {
            auto index = std::type_index(typeid(Component));
            if (!isComponent<Component>())
                throw NoSuchComponent();
            return static_cast<ComponentArray<Component> &>(*_components.at(index)).getComponent(entity);
        }

        /**
         * @brief Retrieves the ComponentArray storing every instance of
         * the given type.
         * @tparam Component The component type.
         * @return Reference to the corresponding ComponentArray.
         */
        template<typename Component>
        [[nodiscard]] ComponentArray<Component> &getComponentArray()
        {
            auto index = std::type_index(typeid(Component));
            if (!isComponent<Component>())
                throw NoSuchComponent();
            return static_cast<ComponentArray<Component> &>(*_components.at(index));
        }

        /**
         * @brief Checks whether an entity has a component of the given
         * type.
         * @tparam Component The component type to check.
         * @param entity The entity to check.
         * @return true if the entity has that component, false
         * otherwise.
         */
        template<typename Component>
        [[nodiscard]] bool hasComponent(Entity entity) const
        {
            auto index = std::type_index(typeid(Component));
            if (!isComponent<Component>())
                return false;
            return _components.at(index)->hasComponent(entity);
        }

        /**
         * @brief Checks whether an entity has a component of the given
         * type.
         * @tparam Component The component type to check.
         * @param entity The entity to check.
         * @return true if the entity has that component, false
         * otherwise.
         */
        template<typename Component>
        [[nodiscard]] bool isComponent() const
        {
            return _components.contains(std::type_index(typeid(Component)));
        }

    private:
        ComponentList _components;
    };

} // namespace RType

#endif /* !COMPONENTMANAGER_HPP_ */
