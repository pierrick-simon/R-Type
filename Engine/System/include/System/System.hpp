/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include "ISystem.hpp"
#include <functional>

namespace RType {

    class ComponentManager;

    /**
     * @brief Applies a callback to every instance of a given Component type.
     * @tparam Component The component type this system operates on.
     */
    template<typename Component>
    class System : public ISystem {
        public:
            /**
             * @brief Constructs a system from a callback.
             * @param func Callback applied to each Component instance during
             * update().
             */
            explicit System(std::function<void(Component &)> func);

            /**
             * @brief Applies the stored callback to every Component
             * instance.
             * @param cm Component manager providing access to the
             * ComponentArray<Component>.
             */
            void update(ComponentManager &cm) override;

        private:
            std::function<void(Component &)> _func;
    };

}

#endif /* !SYSTEM_HPP_ */
