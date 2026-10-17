/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** View
*/

#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <tuple>
#include <type_traits>
#include <functional>
#include "ComponentArray.hpp"
#include "ComponentManager.hpp"

namespace RType {

    /**
     * @brief Number of times Component appears in Components.
     * @tparam Component The type to count.
     * @tparam Components The types to search in.
     */
    template<typename Component, typename... Components>
    constexpr std::size_t duplicateCheck =
        (std::is_same_v<Component, Components> + ...);

    template<typename... Components>
    class View {
        public: 
            explicit View(ComponentManager &cm)
            {
                if (!(cm.isComponent<Components>() && ...))
                    _view = {};
                else
                    _view = std::make_tuple(&cm.getComponentArray<Components>()...);
            }

            void each(std::function<void (Entity, Components &...)> f)
            {
             
                std::apply([&](ComponentArray<Components> *...ca) {
                    if (!(ca && ...))
                        return;
                    iterateSmallest(f, *ca...);
                }, _view);
            }

            static_assert(((duplicateCheck<Components, Components...> == 1) && ...),
                "View: each component type must appear only once");

        private:
            std::tuple<ComponentArray<Components> *...> _view;

            /**
             * @brief Iterates over the smallest of the arrays.
             * @param arrays The ComponentArray of each type, in the order of
             * Components.
             */
            void iterateSmallest(std::function<void (Entity, Components &...)> &f, ComponentArray<Components> &...arrays)
            {
                std::size_t min = std::min({arrays.nbComponent()...});

                (void) ((arrays.nbComponent() == min &&
                            (iterate(f, arrays, arrays...), true)) ||
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
            void iterate(std::function<void (Entity, Components &...)> &f, Array &smallest, ComponentArray<Components> &...arrays)
            {
                for (auto &pack : smallest.getDense()) {
                    if (!(arrays.hasComponent(pack.entity()) && ...))
                        continue;
                    f(pack.entity(), arrays.getComponent(pack.entity())...);
                }
            }
    };

}

#endif /* !VIEW_HPP_ */
