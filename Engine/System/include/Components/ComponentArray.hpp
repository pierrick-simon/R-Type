/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ComponentArray
*/

#ifndef COMPONENTARRAY_HPP_
#define COMPONENTARRAY_HPP_

#include "Entity.hpp"
#include "IComponentArray.hpp"
#include <cstddef>
#include <plf_hive.h>
#include <vector>

namespace RType {

    template<typename Component>
    class ComponentArray : public IComponentArray {
        public:
            using Dense = plf::hive<std::pair<Entity, Component>>;

            template<typename... Args>
            Component &addComponent(Entity entity, Args &&...args);

            void removeComponent(Entity entity) override;
            Component &getComponent(Entity entity);
            bool hasComponent(Entity entity) const override;
            std::size_t nbComponent() const override;

            Dense &getDense();

        private:
            Dense _dense;
            std::vector<typename Dense::iterator> _sparse;
    };

}

#endif /* !COMPONENTARRAY_HPP_ */
