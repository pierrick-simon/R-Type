/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include <functional>
#include <tuple>
#include "ComponentManager.hpp"
#include "ISystem.hpp"

namespace RType {

    /**
     * @brief Applies a callback to every entity that has all the given
     * component types.
     *
     * Iterates over the dense storage of the first type, and skips the
     * entities missing one of the other types.
     * @tparam Components The component types this system operates on. The
     * first one is iterated over: put the rarest one first.
     */
    template<typename... Components>
    class System : public ISystem {
    public:
        /**
         * @brief Constructs a system from a callback.
         * @param func Callback receiving one reference per component type,
         * in the order of Components, during update(). It is moved from.
         */
        explicit System(std::function<void(Components &...)> &func) :
            _func(std::move(func))
        {
        }

        /**
         * @brief Applies the stored callback to every entity that has all
         * the Components.
         * @param cm Component manager providing access to the
         * ComponentArray of each type. Does nothing if one of the types
         * has never been added.
         */
        void update(ComponentManager &cm) override
        {
            if (!(cm.isComponent<Components>() && ...))
                return;
            auto componentArrayList =
                std::tie(cm.getComponentArray<Components>()...);
            auto &firstComponentArray = std::get<0>(componentArrayList);
            for (auto &component : firstComponentArray.getDense()) {
                if (!(cm.hasComponent<Components>(component.entity()) && ...))
                    break;
                _func(std::get<ComponentArray<Components> &>(componentArrayList)
                        .getComponent(component.entity())...);
            }
        }

    private:
        std::function<void(Components &...)> _func;
    };

} // namespace RType

#endif /* !SYSTEM_HPP_ */
