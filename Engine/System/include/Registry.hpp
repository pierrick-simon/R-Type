#ifndef REGISTRY_HPP
    #define REGISTRY_HPP
    #include "ComponentManager.hpp"
    #include "Entity.hpp"

namespace ECS {

    class Registry {
        public:
            Entity createEntity(void);
            void destroyEntity(Entity entity);

            template<typename Component>
            Component &addComponent(Entity entity, Component component);

            template<typename Component>
            void removeComponent(Entity entity);

            template<typename Component>
            Component &getComponent(Entity entity);

            template<typename Component>
            bool hasComponent(Entity entity);

            template<typename Component>
            ComponentArray<Component> &getArray(void);

        private:
            EntityManager mEntityManager;
            ComponentManager mComponentManager;
    };

}
#endif