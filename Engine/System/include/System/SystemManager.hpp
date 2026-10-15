/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#ifndef SYSTEMMANAGER_HPP_
#define SYSTEMMANAGER_HPP_

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include "Exceptions.hpp"
#include "ISystem.hpp"
#include "System.hpp"

namespace RType {

    class ComponentManager;

    /**
     * @brief Id of a system. Ids are never reused.
     */
    using SystemId = std::size_t;

    /**
     * @brief A registered system and its id.
     */
    struct SystemEntry {
        SystemId id;
        std::unique_ptr<ISystem> system;
    };

    /**
     * @brief Collection of every registered system, in execution order.
     */
    using SystemList = std::list<SystemEntry>;

    /**
     * @brief Owns and runs every registered system.
     */
    class SystemManager {
    public:
        explicit SystemManager() = default;

        /**
         * @brief Runs every registered system once, in creation order.
         * @param cm Component manager passed to each system's update().
         */
        void process(ComponentManager &cm);

        /**
         * @brief Creates and registers a new system.
         * @tparam Component The component type the system operates on.
         * @param func Callback applied to each Component instance. It is
         * moved from.
         * @return Id identifying the created system, usable with
         * destroySystem.
         */
        template<typename Component>
        SystemId createSystem(std::function<void(Component &)> &func)
        {
            SystemId id = _nextId++;

            _systems.push_back({id, std::make_unique<System<Component>>(func)});
            return id;
        }

        /**
         * @brief Removes a previously created system.
         * @param id Id returned by createSystem.
         * @throw NoSuchSystem if no system has this id.
         */
        void destroySystem(SystemId id);

        /**
         * @brief Checks whether a system exists or not.
         * @param id The id to check.
         * @return true if the system exists, false otherwise.
         */
        [[nodiscard]] bool hasSystem(SystemId id) const;

    private:
        SystemList _systems;
        SystemId _nextId = 0;
    };

} // namespace RType

#endif /* !SYSTEMMANAGER_HPP_ */
