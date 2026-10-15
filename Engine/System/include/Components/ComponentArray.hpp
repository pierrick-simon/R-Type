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
#include <utility>
#include <vector>

#include "Entity.hpp"
#include "Exceptions.hpp"
#include "IComponentArray.hpp"

namespace RType {

    /**
     * @brief Stores every instance of a given Component type, indexed by
     * Entity (sparse set).
     *
     * The dense storage holds the components themselves and is fast to
     * iterate. The sparse storage maps each entity index to its component
     * in the dense storage, for O(1) access. A component never moves in
     * memory until it is removed.
     * @tparam Component The component type stored by this array.
     */
    template<typename Component>
    class ComponentArray : public IComponentArray {
    public:
        /**
         * @brief Packed storage type holding every component. Iteration
         * order is unspecified.
         */
        using Dense = plf::colony<Component>;

        explicit ComponentArray() = default;

        ComponentArray(const ComponentArray &) = delete;

        ComponentArray &operator=(const ComponentArray &) = delete;

        /**
         * @brief Constructs and stores a component for an entity.
         * @tparam Args Constructor argument types forwarded to Component.
         * @param entity The entity to attach the component to.
         * @param args Arguments forwarded to Component's constructor.
         * @return Reference to the newly created component.
         * @throw AlreadyExistingComponent If the entity already has a
         * component in this array.
         */
        template<typename... Args>
        Component &addEntity(Entity entity, Args &&...args)
        {
            if (hasComponent(entity))
                throw AlreadyExistingComponent();
            auto iter = _dense.emplace(std::forward<Args>(args)...);
            auto sSize = _sparse.size();
            auto eIndex = entity.index();
            if (sSize <= eIndex)
                _sparse.resize(eIndex + (eIndex / 2) + 1);
            _sparse[eIndex] = iter;
            return *iter;
        }

        /**
         * @brief Removes the component associated with an entity.
         * @param entity The entity whose component should be removed.
         * @throw NoSuchComponent If the entity has no component in this
         * array.
         */
        void removeEntity(Entity entity) override
        {
            if (!hasComponent(entity))
                throw NoSuchComponent();
            _dense.erase(_sparse[entity.index()]);
            _sparse[entity.index()] = typename Dense::iterator {};
        }

        /**
         * @brief Retrieves the component associated with an entity.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         * @throw NoSuchComponent If the entity has no component in this
         * array.
         */
        Component &getComponent(Entity entity)
        {
            if (!hasComponent(entity))
                throw NoSuchComponent();
            return *_sparse[entity.index()];
        }

        /**
         * @brief Checks whether an entity has a component in this array.
         * @param entity The entity to check.
         * @return true if the entity has a component here, false
         * otherwise.
         * @note Only the index is checked, not the generation: the
         * caller must check that the entity is alive.
         */
        [[nodiscard]] bool hasComponent(Entity entity) const override
        {
            return (_sparse.size() > entity.index() &&
                _sparse[entity.index()] != typename Dense::iterator {});
        }

        /** @brief Returns the number of components stored. */
        [[nodiscard]] std::size_t nbComponent() const override
        {
            return _dense.size();
        }

        /**
         * @brief Provides direct access to the packed storage.
         * @return Reference to the dense container of components.
         * @warning Removing a component while iterating over it
         * invalidates the iteration.
         */
        Dense &getDense()
        {
            return _dense;
        }

    private:
        Dense _dense;
        std::vector<typename Dense::iterator> _sparse;
    };

} // namespace RType

#endif /* !COMPONENTARRAY_HPP_ */
