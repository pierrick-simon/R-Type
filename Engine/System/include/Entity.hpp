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

    /**
     * @brief Unique identifier for an entity.
     */
    using Entity = std::size_t;

    /**
     * @brief Collection of entity identifiers.
     */
    using EntityList = std::vector<Entity>;

    /**
     * @brief Creates and destroys entities.
     */
    class EntityManager {
    public:
        /**
         * @brief Constructs an EntityManager bound to a ComponentManager.
         * @param cm Component manager used to clean up an entity's
         * components on destruction.
         */
        explicit EntityManager(ComponentManager &cm);

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

    private:
        EntityList _entities;
        ComponentManager &_cm;
    };

} // namespace RType

#endif /* !ENTITY_HPP_ */
