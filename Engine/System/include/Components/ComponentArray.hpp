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
#include "Entity.hpp"
#include "IComponentArray.hpp"

namespace RType {

    /**
     * @brief Stores every instance of a given Component type, indexed by
     * Entity.
     * @tparam Component The component type stored by this array.
     */
    template<typename Component>
    class ComponentArray : public IComponentArray {
    public:
        /**
         * @brief Packed storage type pairing each component with its
         * owning entity.
         */
        using Dense = plf::colony<std::pair<Entity, Component>>;

        /**
         * @brief Constructs and stores a component for an entity.
         * @tparam Args Constructor argument types forwarded to Component.
         * @param entity The entity to attach the component to.
         * @param args Arguments forwarded to Component's constructor.
         * @return Reference to the newly created component.
         */
        template<typename... Args>
        Component &addComponent(Entity entity, Args &&...args);

        /**
         * @brief Removes the component associated with an entity.
         * @param entity The entity whose component should be removed.
         */
        void removeComponent(Entity entity) override;

        /**
         * @brief Retrieves the component associated with an entity.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         */
        Component &getComponent(Entity entity);

        /**
         * @brief Checks whether an entity has a component in this array.
         * @param entity The entity to check.
         * @return true if the entity has a component here, false
         * otherwise.
         */
        [[nodiscard]] bool hasComponent(Entity entity) const override;

        /**
         * @brief Returns the number of components stored.
         * @return Number of components currently stored.
         */
        [[nodiscard]] std::size_t nbComponent() const override;

        /**
         * @brief Provides direct access to the packed storage.
         * @return Reference to the dense container of (Entity, Component)
         * pairs.
         */
        Dense &getDense();

    private:
        Dense _dense;
        std::vector<Dense::iterator> _sparse;
    };

} // namespace RType

#endif /* !COMPONENTARRAY_HPP_ */
