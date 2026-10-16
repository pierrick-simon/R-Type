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
#include <type_traits>

#include "ComponentManager.hpp"
#include "ISystem.hpp"

namespace RType {

    /**
     * @brief Number of times Component appears in Components.
     * @tparam Component The type to count.
     * @tparam Components The types to search in.
     */
    template<typename Component, typename... Components>
    constexpr std::size_t duplicateCheck =
        (std::is_same_v<Component, Components> + ...);

    /**
     * @brief Applies a callback to every entity that has all the given
     * component types.
     *
     * Iterates over the dense storage of the smallest array, and skips
     * the entities missing one of the other types.
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
         * the Components.
         * @param cm Component manager providing access to the
         * ComponentArray of each type. Does nothing if one of the types
         * has never been added.
         */
        void update(ComponentManager &cm) override
        {
            if (!(cm.isComponent<Components>() && ...))
                return;
            iterateSmallest(cm.getComponentArray<Components>()...);
        }

    private:
        std::function<void(Components &...)> _func;

        /**
         * @brief Iterates over the smallest of the arrays.
         * @param arrays The ComponentArray of each type, in the order of
         * Components.
         */
        void iterateSmallest(ComponentArray<Components> &...arrays)
        {
            std::size_t min = std::min({arrays.nbComponent()...});

            (void) ((arrays.nbComponent() == min &&
                        (iterate(arrays, arrays...), true)) ||
                ...);
        }

        /**
         * @brief Calls the callback on every entity of an array that has
         * all the Components.
         * @tparam Array The type of the array iterated over.
         * @param smallest The array iterated over.
         * @param arrays The ComponentArray of each type, in the order of
         * Components.
         */
        template<typename Array>
        void iterate(Array &smallest, ComponentArray<Components> &...arrays)
        {
            for (auto &pack : smallest.getDense()) {
                if (!(arrays.hasComponent(pack.entity()) && ...))
                    continue;
                _func(arrays.getComponent(pack.entity())...);
            }
        }
    };

} // namespace RType

#endif /* !SYSTEM_HPP_ */
