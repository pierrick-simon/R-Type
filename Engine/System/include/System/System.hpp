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

    template<typename Component>
    class System : public ISystem {
        public:
            explicit System(std::function<void(Component &)> func);

            void update(ComponentManager &cm) override;

        private:
            std::function<void(Component &)> _func;
    };

}

#endif /* !SYSTEM_HPP_ */
