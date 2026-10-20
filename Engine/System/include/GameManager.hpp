/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManager
*/

#ifndef GAMEMANAGER_HPP_
#define GAMEMANAGER_HPP_

#include <memory>
#include <type_traits>
#include <utility>

#include "ComponentManager.hpp"
#include "Entity.hpp"
#include "SystemManager.hpp"
#include "View.hpp"

namespace RType {

    /**
     * @brief A deferred structural change (add, remove, destroy...),
     * executed when the CommandQueue is flushed.
     */
    using Command = std::function<void()>;

    /**
     * @brief Commands waiting to be executed, in the order they were
     * requested.
     */
    using CommandQueue = std::vector<Command>;

    /**
     * @brief Facade exposing entity, component and system management.
     *
     * This is the only class the game should use: EntityManager,
     * ComponentManager and SystemManager are internal details.
     *
     * Structural changes (destroyEntity, addComponent, removeComponent,
     * removeComponents, destroySystem) are applied immediately outside
     * update(). During update(), they are queued and applied all together
     * at the end of the frame, so that systems never invalidate the
     * arrays they are iterating over.
     *
     * @warning Components must not call the GameManager from their
     * constructor or destructor: they run while a command is being
     * applied.
     */
    class GameManager {
    public:
        explicit GameManager();

        GameManager(const GameManager &) = delete;

        GameManager &operator=(const GameManager &) = delete;

        /**
         * @brief Runs every registered system once, in creation order,
         * then applies every command queued during the frame.
         *
         * The frame is always applied, even if a system throws. Calling
         * update() from inside a system does nothing.
         *
         * @throw Any exception thrown by a system, rethrown once the
         * frame has been applied. Errors from the queued commands are
         * then discarded.
         * @throw EngineException The first error raised by a queued
         * command, if no system threw.
         */
        void update();

        /**
         * @brief Creates a new entity. Always immediate, even during
         * update().
         *
         * The index of a destroyed entity may be reused, with a higher
         * generation, so old handles never refer to the new entity.
         * @return The newly created entity's handle.
         */
        Entity createEntity();

        /**
         * @brief Destroys an entity and all of its components. Deferred
         * to the end of the frame during update().
         *
         * Does nothing if the entity is already destroyed or the handle
         * is stale.
         * @param entity The entity to destroy.
         */
        void destroyEntity(Entity entity);

        /**
         * @brief Checks whether an entity exists or not.
         * @param entity The entity to check.
         * @return true if the entity is alive and the handle is not
         * stale, false otherwise.
         */
        bool hasEntity(Entity entity) const;

        /**
         * @brief Constructs and stores a component of the given type for
         * an entity. Deferred to the end of the frame during update().
         *
         * The component is constructed immediately: if its constructor
         * throws, the exception is raised right away and nothing is
         * queued.
         * @tparam Component The component type to add.
         * @tparam Args Constructor argument types forwarded to Component.
         * @param entity The entity to attach the component to.
         * @param args Arguments forwarded to Component's constructor.
         * @throw NoSuchEntity If the entity is destroyed or the handle is
         * stale.
         * @throw AlreadyExistingComponent If the entity already has a
         * Component.
         */
        template<typename Component, typename... Args>
        void addComponent(Entity entity, Args &&...args)
        {
            auto component =
                std::make_shared<Component>(std::forward<Args>(args)...);

            _cq.emplace_back([this, entity, component]() {
                if (!hasEntity(entity))
                    throw NoSuchEntity();
                _cm.addComponent<Component>(entity, std::move(*component));
            });
            tryFlush();
        }

        /**
         * @brief Removes a component of the given type from an entity.
         * Deferred to the end of the frame during update().
         * @tparam Component The component type to remove.
         * @param entity The entity to remove the component from.
         * @throw NoSuchEntity If the entity is destroyed or the handle is
         * stale.
         * @throw NoSuchComponent If the entity has no Component.
         */
        template<typename Component>
        void removeComponent(Entity entity)
        {
            _cq.emplace_back([this, entity]() {
                if (!hasEntity(entity))
                    throw NoSuchEntity();
                _cm.removeComponent<Component>(entity);
            });
            tryFlush();
        }

        /**
         * @brief Removes all the components from an entity, without
         * destroying it. Deferred to the end of the frame during update().
         * @param entity The entity to remove the components from.
         * @throw NoSuchEntity If the entity is destroyed or the handle is
         * stale.
         */
        void removeComponents(Entity entity);

        /**
         * @brief Retrieves a component of the given type for an entity.
         *
         * The reference stays valid until this component is removed or
         * its entity destroyed, whatever happens to other components.
         * @tparam Component The component type requested.
         * @param entity The entity whose component is requested.
         * @return Reference to the component.
         * @throw NoSuchEntity If the entity is destroyed or the handle is
         * stale.
         * @throw NoSuchComponent If the entity has no Component.
         */
        template<typename Component>
        [[nodiscard]] Component &getComponent(Entity entity)
        {
            if (!hasEntity(entity))
                throw NoSuchEntity();
            return _cm.getComponent<Component>(entity);
        }

        /**
         * @brief Checks whether an entity has a component of the given
         * type. Never throws.
         * @tparam Component The component type to check.
         * @param entity The entity to check.
         * @return true if the entity is alive and has that component,
         * false otherwise (including for a destroyed or stale entity).
         */
        template<typename Component>
        [[nodiscard]] bool hasComponent(Entity entity) const
        {
            if (!hasEntity(entity))
                return false;
            return _cm.hasComponent<Component>(entity);
        }

        /**
         * @brief Retrieves the ComponentArray storing every instance of
         * the given type.
         * @tparam Component The component type.
         * @return Reference to the corresponding ComponentArray.
         * @throw NoSuchComponent If no Component has ever been added.
         * @warning Outside update(), destroying entities while iterating
         * over its dense storage invalidates the iteration: collect the
         * entities first, then destroy them.
         */
        template<typename Component>
        ComponentArray<Component> &getArray()
        {
            return _cm.getComponentArray<Component>();
        }

        /**
         * @brief Creates and registers a new system. Always immediate.
         *
         * The callback is called once per entity that has all the
         * Components, with one reference per type, in the same order.
         * Without any type (createSystem(func) or createSystem<>(func)),
         * it is called only once per update(), with no argument.
         * @tparam Components The component types the system operates on.
         * Their order does not matter: the smallest array is iterated over.
         * @param func Callback applied to each matching entity.
         * @return Id identifying the created system, usable with
         * destroySystem.
         * @warning Must not be called during update(): the new system
         * would run in the current frame.
         */
        template<typename... Components>
        SystemId createSystem(
            std::type_identity_t<std::function<void(Components &...)>> func)
        {
            return _sm.createSystem<Components...>(func);
        }

        /**
         * @brief Creates and registers a new system whose callback also
         * receives this GameManager. Always immediate.
         *
         * Same as the other overload, but the callback takes the
         * GameManager as its first parameter, which lets systems be
         * written as plain functions instead of capturing it.
         * @tparam Components The component types the system operates on.
         * @param func Callback taking the GameManager, then one reference
         * per component type. It is copied into the system.
         * @return Id identifying the created system, usable with
         * destroySystem.
         * @warning Must not be called during update(): the new system
         * would run in the current frame.
         */
        template<typename... Components>
        SystemId createSystem(const std::type_identity_t<
            std::function<void(GameManager &, Components &...)>> &func)
        {
            std::function<void(Components & ...)> wrapped =
                [this, func](Components &...c) {
                    func(*this, c...);
                };
            return _sm.createSystem<Components...>(wrapped);
        }

        /**
         * @brief Removes a previously created system. Deferred to the end
         * of the frame during update(). Does nothing if the system no
         * longer exists.
         * @param id Id returned by createSystem.
         */
        void destroySystem(SystemId id);

        /**
         * @brief Checks whether a system exists or not.
         * @param id The id to check.
         * @return true if the system exists, false otherwise.
         */
        [[nodiscard]] bool hasSystem(SystemId id) const;

        /**
         * @brief Creates a view over the entities that have all the given
         * component types. Never throws: if one of the types has never
         * been added, the view is empty.
         * @tparam Components The component types to look for.
         * @return The view, to iterate over with each() or getVector().
         */
        template<typename... Components>
        View<Components...> view()
        {
            return View<Components...>(_cm);
        }

        std::unordered_map<int, std::string> &getAssetsMap()
        {
            return _assetsMap;
        }

        void setAssetsMap(std::unordered_map<int, std::string> &map)
        {
            _assetsMap = map;
        }

    private:
        ComponentManager _cm;
        EntityManager _em;
        SystemManager _sm;
        CommandQueue _cq;
        std::unordered_map<int, std::string> _assetsMap;

        bool _updating = false;

        /**
         * @brief Executes every queued command, in order, then empties
         * the queue. A failing command does not stop the following ones.
         * @throw The first exception raised by a command, once every
         * command has run.
         */
        void flush();

        /**
         * @brief Flushes the queue, unless systems are currently running.
         */
        void tryFlush();
    };

} // namespace RType

#endif /* !GAMEMANAGER_HPP_ */
