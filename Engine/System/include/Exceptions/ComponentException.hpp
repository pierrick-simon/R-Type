#ifndef COMPONENTEXCEPTION_HPP
    #define COMPONENTEXCEPTION_HPP
    #include "SystemException.hpp"

namespace ECS {
    class ComponentException : public SystemException {
        public:
            ComponentException(const std::string &error);
    };
}

#endif