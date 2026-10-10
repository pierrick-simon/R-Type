#include "include/Exceptions/ComponentException.hpp"

namespace ECS {
    template<typename Component>
    inline Component &ComponentArray<Component>::insertData(Entity entity, Component component)
    {
        if (hasData(entity)) {
            throw ComponentException("The entity already has data");
        }
        if (entity >= mSparse.size()) {
            mSparse.resize(entity + 1, NULL_INDEX);
        }
        mSparse[entity] = static_cast<std::uint32_t>(mComponentArray.size());
        mComponentArray.push_back(std::move(component));
        mDenseToEntity.push_back(entity);
        return mComponentArray.back();
    }

    template <typename Component>
    inline void ComponentArray<Component>::removeData(Entity entity)
    {
        if (!hasData(entity)) {
            throw ComponentException("The entity does not have the component");
        }
        std::uint32_t index = mSparse[entity];
        std::uint32_t last = static_cast<std::uint32_t>(mComponentArray.size()) - 1;
        Entity lastEntity = mDenseToEntity[last];

        mComponentArray[index] = std::move(mComponentArray[last]);
        mDenseToEntity[index] = lastEntity;
        mSparse[lastEntity] = index;
        mSparse[entity] = NULL_INDEX;
        mComponentArray.pop_back();
        mDenseToEntity.pop_back();
    }

    template <typename Component>
    inline bool ComponentArray<Component>::hasData(Entity entity) const
    {
        return entity < mSparse.size() && mSparse[entity] != NULL_INDEX;
    }

    template <typename Component>
    inline Component &ComponentArray<Component>::getData(Entity entity)
    {
        if (!hasData(entity)) {
            throw ComponentException("The entity does not have the component");
        }
        return mComponentArray[mSparse[entity]];
    }

    template <typename Component>
    inline void ComponentArray<Component>::destroyEntity(Entity entity)
    {
        if (hasData(entity))
            removeData(entity);
    }

    template <typename Component>
    inline std::size_t ComponentArray<Component>::size(void) const
    {
        return mComponentArray.size();
    }

    template <typename Component>
    inline const std::vector<Entity> &ComponentArray<Component>::entities(void) const
    {
        return mDenseToEntity;
    }
}

