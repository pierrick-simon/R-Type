/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Exceptions
*/

#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <exception>
#include <string>

namespace RType {

    class EngineException : public std::exception {
    public:
        EngineException(const std::string &str) :
            _str("Engine Error: " + str) {};

        [[nodiscard]] const char *what() const noexcept override
        {
            return _str.c_str();
        };

    private:
        std::string _str;
    };

        class AlreadyExistingComponent : public EngineException {
    public:
        AlreadyExistingComponent() :
            EngineException(
                "Already an existing component linked to this entity")
        {
        }
    };

    class NoSuchComponent : public EngineException {
    public:
        NoSuchComponent() :
            EngineException("No such component.")
        {
        }

        NoSuchComponent(const std::string &msg) :
            EngineException(msg + " No such component.")
        {
        }
    };

} // namespace RType

#endif /* !EXCEPTIONS_HPP_ */
