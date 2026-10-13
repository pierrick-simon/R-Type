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

}


#endif /* !EXCEPTIONS_HPP_ */
