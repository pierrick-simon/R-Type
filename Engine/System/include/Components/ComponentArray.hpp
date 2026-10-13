/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentArray
*/

#ifndef COMPONENTARRAY_HPP_
#define COMPONENTARRAY_HPP_

#include <cstddef>
#include <plf_colony.h>
#include <vector>
#include <utility>

#include "Entity.hpp"
#include "IComponentArray.hpp"
#include "Exceptions.hpp"

namespace RType {

    class AlreadyExistingComponent;
    
    class NoSuchComponent;

    /**
     * @brief Stores every instance of a given Component type, indexed by
     * Entity.
     * @tparam Component The component type stored by this array.
     */
    template<typename Component>
    class ComponentArray : public IComponentArray {
    public:
        /**
         * @brief Packed storage type with every composents.
         */
        using Dense = plf::colony<Component>;

        /**
         * @brief Default constructor.
         */
        ComponentArray()
        {
        }

        /**
         * @brief Copy construction is disabled.
         */
        ComponentArray(const ComponentArray &) = delete;

        /**
         * @brief Copy assignment is disabled.
         */
        ComponentArray &operator=(const ComponentArray &) = delete;

        /**
         * @brief Constructs and stores a component for an entity.
         * @tparam Args Constructor argument types forwarded to Component.
         * @param entity The entity to attach the component to.
         * @param args Arguments forwarded to Component's constructor.
         * @return Reference to the newly created component.
         */
        template<typename... Args>
        Component &addEntity(Entity entity, Args &&...args)
        {
            if (hasComponent(entity))
                throw AlreadyExistingComponent();
            auto iter = _dense.emplace(std::forward<Args>(args)...);
            auto sSize = _sparse.size();
            if (sSize <= entity)
                _sparse.resize(entity + (entity / 2) + 1);
            _sparse[entity] = iter;
            return *iter;
        }

        /**
         * @brief Removes the component associated with an entity.
         * @param entity The entity whose component should be removed.
         */
        void removeEntity(Entity entity) override
        {
            if (hasComponent(entity))
                throw NoSuchComponent();
            _dense.erase(_sparse[entity]);
            _sparse[entity] = typename Dense::iterator{};
        }

        /**
         * @brief Retrieves the component associated with an entity.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         */
        Component &getComponent(Entity entity)
        {
            if (hasComponent(entity))
                throw NoSuchComponent();
            return _sparse[entity];
        }

        /**
         * @brief Checks whether an entity has a component in this array.
         * @param entity The entity to check.
         * @return true if the entity has a component here, false
         * otherwise.
         */
        [[nodiscard]] bool hasComponent(Entity entity) const override
        {
            return (_sparse.size() > entity && _sparse[entity] != typename Dense::iterator{});
        }

        /**
         * @brief Returns the number of components stored.
         * @return Number of components currently stored.
         */
        [[nodiscard]] std::size_t nbComponent() const override
        {
            return _dense.size();
        }

        /**
         * @brief Provides direct access to the packed storage.
         * @return Reference to the dense container of Component
         * pairs.
         */
        Dense &getDense()
        {
            return _dense;
        }

    private:
        Dense _dense;
        std::vector<typename Dense::iterator> _sparse;
    };

    class AlreadyExistingComponent: public EngineException {
        public:
            AlreadyExistingComponent(): EngineException("Already an existing component linked to this entity") {}
    };

    class NoSuchComponent: public EngineException {
        public:
            NoSuchComponent(): EngineException("No such component.") {}
            
            NoSuchComponent(const std::string &msg): EngineException(msg + " No such component.") {}
    };

} // namespace RType

#endif /* !COMPONENTARRAY_HPP_ */
