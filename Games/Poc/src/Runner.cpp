/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Runner
*/

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string_view>

#include "Test.hpp"

#ifndef _WIN32
    #include <sys/wait.h>
    #include <unistd.h>
#endif

namespace {

    constexpr int PASSED = 0;
    constexpr int FAILED = 3;
    constexpr std::string_view UNSUPPORTED = "[unsupported]";

    int runInProcess(const Poc::Test &test)
    {
        try {
            test.func();
            std::cout << "[  OK  ] " << test.name << std::endl;
            return PASSED;
        } catch (const Poc::Failure &failure) {
            std::cout << "[ FAIL ] " << test.name << "\n         "
                      << failure.what() << std::endl;
        } catch (const std::exception &e) {
            std::cout << "[ FAIL ] " << test.name
                      << "\n         unexpected exception: " << e.what()
                      << Poc::throwSite(e) << std::endl;
        } catch (...) {
            std::cout << "[ FAIL ] " << test.name
                      << "\n         unknown exception" << std::endl;
        }
        return FAILED;
    }

#ifndef _WIN32
    int runIsolated(const Poc::Test &test)
    {
        std::cout.flush();
        pid_t pid = fork();

        if (pid < 0)
            return runInProcess(test);
        if (pid == 0) {
            int status = runInProcess(test);
            std::cout.flush();
            std::_Exit(status);
        }
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) &&
            (WEXITSTATUS(status) == PASSED || WEXITSTATUS(status) == FAILED))
            return WEXITSTATUS(status);
        std::cout << "[CRASH ] " << test.name << "\n         ";
        if (WIFSIGNALED(status))
            std::cout << "killed by signal " << WTERMSIG(status) << " ("
                      << strsignal(WTERMSIG(status)) << ")";
        else
            std::cout << "exited with code " << WEXITSTATUS(status)
                      << " (sanitizer report above)";
        std::cout << std::endl;
        return FAILED;
    }
#endif

} // namespace

namespace Poc {

    int runTests(const TestList &tests)
    {
        std::size_t failed = 0;
        std::size_t skipped = 0;

        for (const auto &test : tests) {
            if (test.name.starts_with(UNSUPPORTED)) {
                std::cout << "[ SKIP ] " << test.name << std::endl;
                skipped++;
                continue;
            }
#ifndef _WIN32
            if (runIsolated(test) != PASSED)
                failed++;
#else
            if (runInProcess(test) != PASSED)
                failed++;
#endif
        }
        std::cout << "\n"
                  << tests.size() - failed - skipped << "/"
                  << tests.size() - skipped << " tests passed, " << skipped
                  << " unsupported skipped" << std::endl;
        return failed == 0 ? 0 : 1;
    }

} // namespace Poc
