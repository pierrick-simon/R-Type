/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SystemManager
*/

#ifndef SYSTEMMANAGER_HPP_
#define SYSTEMMANAGER_HPP_

#include "ISystem.hpp"
#include <functional>
#include <memory>
#include <plf_hive.h>

namespace RType {

    class ComponentManager;

    using SystemList = plf::hive<std::unique_ptr<ISystem>>;

    class SystemManager {
        public:
            void process(ComponentManager &cm);

            template<typename Component>
            SystemList::iterator createSystem(std::function<void(Component &)> func);

            void destroySystem(SystemList::iterator system);

        private:
            SystemList _systems;
    };

}

#endif /* !SYSTEMMANAGER_HPP_ */
