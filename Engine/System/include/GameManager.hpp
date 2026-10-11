/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** GameManager
*/

#ifndef GAMEMANAGER_HPP_
#define GAMEMANAGER_HPP_

#include "Components/ComponentManager.hpp"
#include "Entity.hpp"
#include "System/SystemManager.hpp"

namespace RType {

    class GameManager {
        public:
            GameManager();

            GameManager(const GameManager &) = delete;
            GameManager &operator=(const GameManager &) = delete;

            void update();

            Entity createEntity();
            void destroyEntity(Entity entity);

            template<typename Component, typename... Args>
            Component &addComponent(Entity entity, Args &&...args);

            template<typename Component>
            void removeComponent(Entity entity);

            template<typename Component>
            Component &getComponent(Entity entity);

            template<typename Component>
            bool hasComponent(Entity entity) const;

            template<typename Component>
            ComponentArray<Component> &getArray();

            template<typename Component>
            SystemList::iterator createSystem(std::function<void(Component &)> func);

            void destroySystem(SystemList::iterator system);

        private:
            ComponentManager _cm;
            EntityManager _em;
            SystemManager _sm;
    };

}

#endif /* !GAMEMANAGER_HPP_ */
