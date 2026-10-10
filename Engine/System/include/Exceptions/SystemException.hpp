#ifndef SYSTEMEXCEPTION_HPP
    #define SYSTEMEXCEPTION_HPP
    #include <exception>
    #include <string>

namespace ECS {
    class SystemException : public std::exception {
        public:
            SystemException(const std::string &error);
            const char *what(void) const noexcept;

        private:
            std::string _error;
    };
}


#endif