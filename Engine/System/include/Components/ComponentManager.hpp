#ifndef COMPONENTMANAGER_HPP
    #define COMPONENTMANAGER_HPP
    #include "ComponentArray.hpp"
    #include <memory>

namespace ECS {
    class ComponentManager {
        public:
            template<typename Component>
            Component &addComponent(Entity entity, Component component);

            template<typename Component>
            void removeComponent(Entity entity);
            
            template<typename Component>
            Component &getComponent(Entity entity);

            template<typename Component>
            bool hasComponent(Entity entity);

            void entityDestroyed(Entity entity);
            
            template<typename Component>
            ComponentArray<Component> &getArray(void);

        private:
            std::vector<std::unique_ptr<IComponentArray>> _componentArrays;
    };
    
    #include "ComponentManager.tpp"
}

#endif