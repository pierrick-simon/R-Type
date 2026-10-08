#ifndef COMPONENT_HPP
    #define COMPONENT_HPP
    #include "Entity.hpp"
    #include <vector>
    #define NULL_INDEX UINT32_MAX

class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual bool hasData(Entity entity) const = 0;
        virtual void destroyEntity(Entity entity) = 0;
};

template<typename Component>
class ComponentArray : public IComponentArray {
    public:
        void insertData(Entity entity, Component component);
        bool hasData(Entity entity) const override;
        Component &getData(Entity entity);
        void destroyEntity(Entity entity) override;
        std::size_t size(void) const;
        const std::vector<Entity> &entities(void) const;

    private:
        std::vector<Component> mComponentArray;
        std::vector<Entity> mDenseToEntity;
        std::vector<uint32_t> mSparse;
};

#endif