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

    class IComponentArray {
        public:
            virtual ~IComponentArray() = default;

            virtual std::size_t nbComponent() const = 0;
            virtual void removeComponent(Entity entity) = 0;
            virtual bool hasComponent(Entity entity) const = 0;
    };

}

#endif /* !ICOMPONENTARRAY_HPP_ */
