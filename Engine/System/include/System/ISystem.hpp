/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ISystem
*/

#ifndef ISYSTEM_HPP_
#define ISYSTEM_HPP_

namespace RType {

    class ComponentManager;

    class ISystem {
        public:
            virtual ~ISystem() = default;

            virtual void update(ComponentManager &cm) = 0;
    };

}

#endif /* !ISYSTEM_HPP_ */
