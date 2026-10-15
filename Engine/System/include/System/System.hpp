/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** System
*/

#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

#include <functional>
#include "ComponentManager.hpp"
#include "ISystem.hpp"

namespace RType {

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
         * update(). It is moved from.
         */
        explicit System(std::function<void(Component &)> &func) :
            _func(std::move(func))
        {
        }

        /**
         * @brief Applies the stored callback to every Component
         * instance.
         * @param cm Component manager providing access to the
         * ComponentArray<Component>. Does nothing if no Component has
         * ever been added.
         */
        void update(ComponentManager &cm) override
        {
            if (!cm.isComponent<Component>())
                return;
            auto &dense = cm.getComponentArray<Component>().getDense();
            for (auto &component : dense)
                _func(component);
        }

    private:
        /**
         * @brief Callback applied to each Component instance.
         */
        std::function<void(Component &)> _func;
    };

} // namespace RType

#endif /* !SYSTEM_HPP_ */
