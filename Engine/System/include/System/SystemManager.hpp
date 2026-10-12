/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#ifndef SYSTEMMANAGER_HPP_
#define SYSTEMMANAGER_HPP_

#include <functional>
#include <memory>
#include <plf_colony.h>
#include "ISystem.hpp"

namespace RType {

    class ComponentManager;

    /**
     * @brief Collection of every registered system.
     */
    using SystemList = plf::colony<std::unique_ptr<ISystem>>;

    /**
     * @brief Owns and runs every registered system.
     */
    class SystemManager {
    public:
        /**
         * @brief Runs every registered system once.
         * @param cm Component manager passed to each system's update().
         */
        void process(ComponentManager &cm);

        /**
         * @brief Creates and registers a new system.
         * @tparam Component The component type the system operates on.
         * @param func Callback applied to each Component instance.
         * @return Handle identifying the created system, usable with
         * destroySystem.
         */
        template<typename Component>
        SystemList::iterator createSystem(
            std::function<void(Component &)> func);

        /**
         * @brief Removes a previously created system.
         * @param system Handle returned by createSystem.
         */
        void destroySystem(SystemList::iterator system);

    private:
        SystemList _systems;
    };

} // namespace RType

#endif /* !SYSTEMMANAGER_HPP_ */
