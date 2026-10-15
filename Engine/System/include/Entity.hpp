/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Entity
*/

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include <cstddef>
#include <queue>
#include <vector>

#include "Exceptions.hpp"

namespace RType {

    /**
     * @brief Generational handle identifying an entity.
     *
     * The index is the entity's slot, reused once the entity is
     * destroyed. The generation is incremented each time the slot is
     * reused, so a handle kept after its entity's destruction (a stale
     * handle) never refers to the new entity of the same slot.
     */
    class Entity {
    public:
        /**
         * @brief Constructs a handle from its index and generation.
         * @param index The entity's slot.
         * @param generation How many times the slot has been reused.
         * @warning Handles should only come from createEntity(): a
         * hand-made handle may refer to an entity that is not yours.
         */
        explicit Entity(std::size_t index, std::size_t generation = 0);

        /**
         * @brief Compares two handles.
         * @param entity The handle to compare with.
         * @return true if both the index and the generation are equal.
         */
        [[nodiscard]] bool operator==(const Entity &entity) const;

        /** @brief Returns the entity's slot, used to index the arrays. */
        [[nodiscard]] std::size_t index() const;

        /** @brief Returns how many times the entity's slot has been reused. */
        [[nodiscard]] std::size_t generation() const;

    private:
        std::size_t _index;
        std::size_t _generation;
    };

    /**
     * @brief State of an entity slot.
     */
    struct EntityInfo {
        bool isActive = true;
        std::size_t generation = 0;
    };

    /**
     * @brief State of every entity slot, indexed by Entity::index().
     */
    using EntityList = std::vector<EntityInfo>;

    /**
     * @brief Destroyed entities whose slot can be reused, oldest first.
     */
    using RecycleQueue = std::queue<Entity>;

    /**
     * @brief Creates, destroys and tracks the liveness of entities.
     */
    class EntityManager {
    public:
        explicit EntityManager() = default;

        EntityManager(const EntityManager &) = delete;

        EntityManager &operator=(const EntityManager &) = delete;

        /**
         * @brief Creates a new entity, reusing the slot of the oldest
         * destroyed entity if any, with its generation incremented.
         * @return The newly created entity's handle.
         */
        Entity createEntity();

        /**
         * @brief Marks an entity as destroyed and queues its slot for
         * reuse. Does not touch its components.
         * @param entity The entity to destroy.
         * @throw NoSuchEntity If the entity is already destroyed or the
         * handle is stale.
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Checks whether an entity exists or not.
         * @param entity The entity to check.
         * @return true if the slot is active and its generation matches
         * the handle's, false otherwise.
         */
        [[nodiscard]] bool hasEntity(Entity entity) const;

    private:
        EntityList _entities;
        RecycleQueue _toRecycle;
    };

} // namespace RType

#endif /* !ENTITY_HPP_ */
