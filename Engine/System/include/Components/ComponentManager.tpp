#ifndef TEST
#include "ComponentManager.hpp"
#endif

namespace ECS {
    template <typename Component>
    inline Component &ComponentManager::addComponent(Entity entity, Component component)
    {
        return getArray<Component>().insertData(entity, std::move(component));
    }

    template <typename Component>
    inline void ComponentManager::removeComponent(Entity entity)
    {
        getArray<Component>().removeData(entity);
    }

    template <typename Component>
    inline Component &ComponentManager::getComponent(Entity entity)
    {
        return getArray<Component>().getData(entity);
    }

    template <typename Component>
    inline bool ComponentManager::hasComponent(Entity entity)
    {
        return getArray<Component>().hasData(entity);
    }

    template <typename Component>
    inline ComponentArray<Component> &ComponentManager::getArray(void)
    {
        // TODO: insert return statement here
        return _componentArrays[0];
    }
}