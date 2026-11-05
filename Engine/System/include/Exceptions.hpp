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
            _message(str), _str("Engine Error: " + str) {};

        /** @brief Returns the description of the error. */
        [[nodiscard]] const char *what() const noexcept override
        {
            return _str.c_str();
        };

        /**
         * @brief Returns the description of the error, without the
         * "Engine Error: " prefix.
         */
        [[nodiscard]] const std::string &message() const noexcept
        {
            return _message;
        };

    private:
        std::string _message;
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

    /**
     * @brief Raised when a user try a non existing module.
     */
    class NoSuchModule : public EngineException {
    public:
        NoSuchModule(const std::string &module) :
            EngineException("No such module: " + module)
        {
        }
    };

    /**
     * @brief Raised when a module is used without a module it depends on.
     */
    class MissingModule : public EngineException {
    public:
        MissingModule(
            const std::string &module, const std::string &dependency) :
            EngineException(
                "Module " + module + " requires module " + dependency)
        {
        }
    };

    /**
     * @brief Raised when a Lua script fails to load or one of its
     * functions raises an error.
     */
    class ScriptError : public EngineException {
    public:
        /**
         * @param msg The error reported by Lua.
         */
        ScriptError(const std::string &msg) :
            EngineException("Script error: " + msg)
        {
        }
    };

    /**
     * @brief Raised when a script asks for an engine system that does not
     * exist.
     */
    class UnknownSystem : public EngineException {
    public:
        UnknownSystem(const std::string &system) :
            EngineException("Unknown system: " + system)
        {
        }
    };

    /**
     * @brief Raised when a script asks for an engine component that does
     * not exist.
     */
    class UnknownComponent : public EngineException {
    public:
        UnknownComponent(const std::string &component) :
            EngineException("Unknown component: " + component)
        {
        }
    };

    /**
     * @brief Raised when a script gives a component a field that does not
     * exist, a value of the wrong type, or leaves out a required field.
     */
    class InvalidComponentArgs : public EngineException {
    public:
        InvalidComponentArgs(
            const std::string &component, const std::string &reason) :
            EngineException(
                "Invalid arguments for " + component + ": " + reason)
        {
        }
    };

    /**
     * @brief Raised when the list of components a script gives to a system
     * holds something else than a name, or the same name twice.
     */
    class InvalidComponentList : public EngineException {
    public:
        InvalidComponentList(const std::string &reason) :
            EngineException("Invalid component list: " + reason)
        {
        }
    };

    /**
     * @brief Raised when a script creates a system from a system, instead
     * of from start.
     */
    class SystemCreatedInSystem : public EngineException {
    public:
        /**
         * @param function The function the script called.
         */
        SystemCreatedInSystem(const std::string &function) :
            EngineException(function
                + " cannot be called from a system, only from start")
        {
        }
    };

} // namespace RType

#endif /* !EXCEPTIONS_HPP_ */
