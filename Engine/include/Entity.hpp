#ifndef ENTITY_HPP
    #define ENTITY_HPP
    #include <cstdint>
    #include <vector>

using Entity = std::uint32_t;

class EntityManager {
    public:
        Entity create(void);
        void release(Entity entity);
    
    private:
        std::vector<Entity> mFree;
        Entity mNextEntity;
};

#endif