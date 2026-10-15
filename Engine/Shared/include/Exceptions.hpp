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

    /**
     * @brief Base class of every error raised by the engine.
     */
    class EngineException : public std::exception {
    public:
        /**
         * @brief Constructs an error.
         * @param str Description of the error, prefixed with
         * "Engine Error: ".
         */
        EngineException(const std::string &str) :
            _str("Engine Error: " + str) {};

        /** @brief Returns the description of the error. */
        [[nodiscard]] const char *what() const noexcept override
        {
            return _str.c_str();
        };

    private:
        std::string _str;
    };

    /**
     * @brief Raised when adding a component to an entity that already has
     * one of the same type.
     */
    class AlreadyExistingComponent : public EngineException {
    public:
        AlreadyExistingComponent() :
            EngineException(
                "Already an existing component linked to this entity")
        {
        }
    };

    /**
     * @brief Raised when accessing or removing a component an entity does
     * not have, or a component type that has never been added.
     */
    class NoSuchComponent : public EngineException {
    public:
        NoSuchComponent() :
            EngineException("No such component.")
        {
        }

        /**
         * @brief Constructs the error with some context.
         * @param msg Context prepended to the description.
         */
        NoSuchComponent(const std::string &msg) :
            EngineException(msg + " No such component.")
        {
        }
    };

    /**
     * @brief Raised when using an entity that is destroyed, or a stale
     * handle.
     */
    class NoSuchEntity : public EngineException {
    public:
        NoSuchEntity() :
            EngineException("No such entity.")
        {
        }
    };

    /**
     * @brief Raised when destroying a system that does not exist.
     */
    class NoSuchSystem : public EngineException {
    public:
        NoSuchSystem() :
            EngineException("No such system.")
        {
        }
    };

} // namespace RType

#endif /* !EXCEPTIONS_HPP_ */
