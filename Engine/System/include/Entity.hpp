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
#include <queue>

#include "Exceptions.hpp"

namespace RType {

    /**
     * @brief Unique identifier for an entity.
     */
    using Entity = std::size_t;

    /**
     * @brief Collection of entity identifiers.
     */
    using EntityList = std::vector<bool>;

    /**
     * @brief Collection of entity's id to recycle.
     */
    using RecycleQueue = std::queue<Entity>;

    /**
     * @brief Creates and destroys entities.
     */
    class EntityManager {
    public:
        /**
         * @brief Constructs an empty EntityManager.
         */
        explicit EntityManager() = default;

        /**
         * @brief Copy construction is disabled.
         */
        EntityManager(const EntityManager &) = delete;

        /**
         * @brief Copy assignment is disabled.
         */
        EntityManager &operator=(const EntityManager &) = delete;

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
         * @brief Checks whether an entity exist or not.
         * @param entity The entity to check.
         */
        [[nodiscard]] bool hasEntity(Entity entity) const;

    private:
        EntityList _entities;
        RecycleQueue _toRecycle;
    };

} // namespace RType

#endif /* !ENTITY_HPP_ */
