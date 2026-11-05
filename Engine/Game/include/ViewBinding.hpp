/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ViewBinding
*/

#ifndef VIEWBINDING_HPP_
#define VIEWBINDING_HPP_

#include <cstddef>
#include <vector>
#include <sol/sol.hpp>

#include "ComponentBinding.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Lets scripts loop over entities by their components, anywhere.
     */
    class ViewBinding {
    public:
        ViewBinding() = delete;

        /**
         * @brief Lets scripts loop over the entities that have all the
         * given components, from start or from a system:
         * for entity, position, velocity in
         *     gm:view({"Position2D", "Velocity2D"}) do ... end
         *
         * The components are given in the list's order, and are the real
         * ones. The entities are listed when the loop starts; one that is
         * destroyed or loses a component during the loop is skipped, so
         * that the loop can destroy entities even outside update(). An
         * empty list loops over nothing.
         * @throw UnknownComponent If a name is not a component.
         * @throw InvalidComponentList If the list holds something else than
         * a name, or the same name twice.
         */
        static void registerFunctions(sol::state &lua);

    private:
        /**
         * @brief Where a gm:view loop is: shared by every copy sol makes
         * of its iterator.
         */
        struct State {
            GameManager &gm;
            std::vector<const ComponentBinding::Entry *> entries;
            std::vector<Entity> entities;
            std::size_t next = 0;
        };
    };

} // namespace RType::Game

#endif /* !VIEWBINDING_HPP_ */
