#include "ComponentException.hpp"

namespace ECS {
    ComponentException::ComponentException(const std::string &error) :
        SystemException(error)
    {
    }
}
