/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include <algorithm>
#include <functional>

#include "ComponentManager.hpp"
#include "ISystem.hpp"
#include "View.hpp"

namespace RType {

    /**
     * @brief Applies a callback to every entity that has all the given
     * component types.
     *
     * Loops over a View of the Components. Without any type, the system
     * is a frame system: its callback is called only once per update().
     * @tparam Components The component types this system operates on.
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

        static_assert(((duplicateCheck<Components, Components...> == 1) && ...),
            "System: each component type must appear only once");

        /**
         * @brief Applies the stored callback to every entity that has all
         * the Components, or calls it once if there is no Component.
         * @param cm Component manager providing access to the
         * ComponentArray of each type. Does nothing if one of the types
         * has never been added.
         */
        void update(ComponentManager &cm) override
        {
            if constexpr (sizeof...(Components) == 0) {
                _func();
            } else {
                View<Components...>(cm).each(
                    [&](Entity, Components &...c) { _func(c...); });
            }
        }

    private:
        std::function<void(Components &...)> _func;
    };

} // namespace RType

#endif /* !SYSTEM_HPP_ */
