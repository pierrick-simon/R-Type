/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity
*/

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include <cstddef>
#include <vector>

namespace RType {

    class ComponentManager;

    using Entity = std::size_t;
    using EntityList = std::vector<Entity>;

    class EntityManager {
        public:
            explicit EntityManager(ComponentManager &cm);

            Entity createEntity();
            void destroyEntity(Entity entity);

        private:
            EntityList _entities;
            ComponentManager &_cm;
    };

}

#endif /* !ENTITY_HPP_ */
