#ifndef ENTITY_HPP
    #define ENTITY_HPP
    #include <cstdint>
    #include <vector>

namespace ECS {

    using Entity = std::uint32_t;

    /*
    struct Entity
    {
        std::uint32_t entity;
        std::uint32_t generation;
    }
    */

    class EntityManager {
        public:
            Entity create(void);
            void release(Entity entity);
            //getComponents(Entity entity);

        private:
            std::vector<Entity> mFree;
            Entity mNextEntity;
    };

}

#endif