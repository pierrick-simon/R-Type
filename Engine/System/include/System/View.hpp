/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** View
*/

#ifndef VIEW_HPP_
#define VIEW_HPP_

#include <functional>
#include <tuple>
#include <type_traits>

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

    /**
     * @brief Iterates over the entities that have all the given component
     * types, providing each entity with its components.
     *
     * Iterates over the dense storage of the smallest array, and skips the
     * entities missing one of the other types. A view stores no entity: it
     * always sees the current state, and is meant to be used right away.
     * @tparam Components The component types to look for.
     */
    template<typename... Components>
    class View {
    public:
        /**
         * @brief Constructs a view over the arrays of a component
         * manager. Never throws: if one of the types has never been
         * added, the view is empty.
         * @param cm Component manager holding the arrays.
         */
        explicit View(ComponentManager &cm)
        {
            if (!(cm.isComponent<Components>() && ...))
                _view = {};
            else
                _view = std::make_tuple(&cm.getComponentArray<Components>()...);
        }

        /**
         * @brief Calls a function on every entity that has all the
         * Components. Does nothing if one of the types has never been
         * added.
         * @param f Function receiving the entity, then one reference
         * per component type, in the order of Components.
         * @warning Outside update(), destroying entities during the
         * iteration invalidates it: collect them first.
         */
        void each(std::function<void(Entity, Components &...)> f)
        {
            std::apply(
                [&](ComponentArray<Components> *...ca) {
                    if (!(ca && ...))
                        return;
                    iterateSmallest(f, *ca...);
                },
                _view);
        }

        std::vector<std::tuple<Entity, Components &...>> getVector()
        {
            std::vector<std::tuple<Entity, Components &...>> v;
            each([&v] (Entity e, Components &... c) {
                v.emplace_back(e, c...);
            });
            return v;
        }

        static_assert(((duplicateCheck<Components, Components...> == 1) && ...),
            "View: each component type must appear only once");

    private:
        std::tuple<ComponentArray<Components> *...> _view;

        /**
         * @brief Iterates over the smallest of the arrays.
         * @param f Function to call on each matching entity.
         * @param arrays The ComponentArray of each type, in the order of
         * Components.
         */
        void iterateSmallest(std::function<void(Entity, Components &...)> &f,
            ComponentArray<Components> &...arrays)
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
         * @param f Function to call on each matching entity.
         * @param smallest The array iterated over.
         * @param arrays The ComponentArray of each type, in the order of
         * Components.
         */
        template<typename Array>
        void iterate(std::function<void(Entity, Components &...)> &f,
            Array &smallest, ComponentArray<Components> &...arrays)
        {
            for (auto &pack : smallest.getDense()) {
                if (!(arrays.hasComponent(pack.entity()) && ...))
                    continue;
                f(pack.entity(), arrays.getComponent(pack.entity())...);
            }
        }
    };

} // namespace RType

#endif /* !VIEW_HPP_ */
