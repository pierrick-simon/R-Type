/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Test
*/

#ifndef POC_TEST_HPP_
#define POC_TEST_HPP_

#include <exception>
#include <functional>
#include <string>
#include <vector>

namespace Poc {

    class Failure : public std::exception {
    public:
        Failure(const char *file, int line, const std::string &msg) :
            _msg(std::string(file) + ":" + std::to_string(line) + ": " + msg)
        {
        }

        [[nodiscard]] const char *what() const noexcept override
        {
            return _msg.c_str();
        }

    private:
        std::string _msg;
    };

    struct Test {
        std::string name;
        std::function<void()> func;
    };

    using TestList = std::vector<Test>;

    TestList edgeCaseTests();
    TestList simulationTests();
    TestList systemTests();
    TestList multiSystemTests();
    TestList viewTests();
    TestList frameSystemTests();
    TestList gmSystemTests();
    TestList recyclingTests();
    TestList trickyTests();

    int runTests(const TestList &tests);

    std::string throwSite(const std::exception &exception);

} // namespace Poc

#define POC_CHECK(cond)                                                        \
    do {                                                                       \
        if (!(cond))                                                           \
            throw ::Poc::Failure(__FILE__, __LINE__, "CHECK(" #cond ")");      \
    } while (0)

#define POC_CHECK_THROWS(expr, Type)                                           \
    do {                                                                       \
        try {                                                                  \
            static_cast<void>(expr);                                           \
        } catch (const Type &) {                                               \
            break;                                                             \
        } catch (const std::exception &e) {                                    \
            throw ::Poc::Failure(__FILE__,                                     \
                __LINE__,                                                      \
                #expr " threw something else than " #Type ": " +               \
                    std::string(e.what()) + ::Poc::throwSite(e));              \
        }                                                                      \
        throw ::Poc::Failure(                                                  \
            __FILE__, __LINE__, #expr " did not throw " #Type);                \
    } while (0)

#define POC_CHECK_NOTHROW(expr)                                                \
    do {                                                                       \
        try {                                                                  \
            static_cast<void>(expr);                                           \
        } catch (const std::exception &e) {                                    \
            throw ::Poc::Failure(__FILE__,                                     \
                __LINE__,                                                      \
                #expr " threw: " + std::string(e.what()) +                     \
                    ::Poc::throwSite(e));                                      \
        }                                                                      \
    } while (0)

#endif /* !POC_TEST_HPP_ */
