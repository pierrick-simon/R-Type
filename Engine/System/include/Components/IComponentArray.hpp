/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** IComponentArray
*/

#ifndef ICOMPONENTARRAY_HPP_
#define ICOMPONENTARRAY_HPP_

#include "Entity.hpp"
#include <cstddef>

namespace RType {

    /**
     * @brief Type-erased base class for ComponentArray<Component>.
     */
    class IComponentArray {
        public:
            /**
             * @brief Default virtual destructor.
             */
            virtual ~IComponentArray() = default;

            /**
             * @brief Returns the number of components stored.
             * @return Number of components currently stored.
             */
            virtual std::size_t nbComponent() const = 0;

            /**
             * @brief Removes the component associated with an entity, if any.
             * @param entity The entity whose component should be removed.
             */
            virtual void removeComponent(Entity entity) = 0;

            /**
             * @brief Checks whether an entity has a component in this array.
             * @param entity The entity to check.
             * @return true if the entity has a component here, false
             * otherwise.
             */
            virtual bool hasComponent(Entity entity) const = 0;
    };

}

#endif /* !ICOMPONENTARRAY_HPP_ */
