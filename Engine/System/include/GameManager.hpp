/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManager
*/

#ifndef GAMEMANAGER_HPP_
#define GAMEMANAGER_HPP_

#include "ComponentManager.hpp"
#include "Entity.hpp"
#include "SystemManager.hpp"

namespace RType {

    /**
     * @brief Facade exposing entity, component and system management.
     */
    class GameManager {
    public:
        /**
         * @brief Constructs an empty GameManager.
         */
        GameManager();

        /**
         * @brief Copy construction is disabled.
         */
        GameManager(const GameManager &) = delete;

        /**
         * @brief Copy assignment is disabled.
         */
        GameManager &operator=(const GameManager &) = delete;

        /**
         * @brief Runs every registered system once.
         */
        void update();

        /**
         * @brief Creates a new entity.
         * @return The newly created entity's identifier.
         */
        Entity createEntity();

        /**
         * @brief Destroys an entity and all of its components.
         * @param entity The entity to destroy.
         */
        void destroyEntity(Entity entity);

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
            _cm.addComponent<Component, Args...>(entity, args...);
        }

        /**
         * @brief Removes a component of the given type from an entity.
         * @tparam Component The component type to remove.
         * @param entity The entity to remove the component from.
         */
        template<typename Component>
        void removeComponent(Entity entity)
        {
            _cm.removeComponent<Component>(entity);
        }

        /**
         * @brief Retrieves a component of the given type for an entity.
         * @tparam Component The component type requested.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         */
        template<typename Component>
        [[nodiscard]] Component &getComponent(Entity entity)
        {
            return _cm.getComponent<Component>(entity);
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
            return _cm.hasComponent<Component>(entity);
        }

        /**
         * @brief Retrieves the ComponentArray storing every instance of
         * the given type.
         * @tparam Component The component type.
         * @return Reference to the corresponding ComponentArray.
         */
        template<typename Component>
        ComponentArray<Component> &getArray()
        {
            return _cm.getComponentArray<Component>();
        }

        /**
         * @brief Creates and registers a new system.
         * @tparam Component The component type the system operates on.
         * @param func Callback applied to each Component instance.
         * @return Handle identifying the created system, usable with
         * destroySystem.
         */
        template<typename Component>
        SystemList::iterator createSystem(std::function<void(Component &)> func)
        {
            return _sm.createSystem<Component>(func);
        }

        /**
         * @brief Removes a previously created system.
         * @param system Handle returned by createSystem.
         */
        void destroySystem(SystemList::iterator system);

    private:
        ComponentManager _cm;
        EntityManager _em;
        SystemManager _sm;
    };

} // namespace RType

#endif /* !GAMEMANAGER_HPP_ */
