#include "SystemException.hpp"

namespace ECS {
    SystemException::SystemException(const std::string &error)
    {
        _error = error;
    }

    const char *SystemException::what(void) const noexcept
    {
        return _error;
    }
}