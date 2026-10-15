/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ThrowTrace
*/

#include <array>
#include <string>
#include <typeinfo>
#include <version>

#include "Test.hpp"

#if defined(POC_STACKTRACE) && defined(__cpp_lib_stacktrace)

    #include <cxxabi.h>
    #include <dlfcn.h>
    #include <stacktrace>

namespace {

    struct ThrowRecord {
        const void *object = nullptr;
        std::stacktrace trace {};
    };

    constexpr std::size_t HISTORY = 64;
    std::array<ThrowRecord, HISTORY> records;
    std::size_t next = 0;
    bool recording = false;

    void record(const void *object)
    {
        if (recording)
            return;
        recording = true;
        records[next % HISTORY] = {object, std::stacktrace::current(2)};
        next++;
        recording = false;
    }

    const std::stacktrace *find(const void *object)
    {
        for (std::size_t i = 0; i < HISTORY && i < next; i++) {
            const ThrowRecord &r = records[(next - 1 - i) % HISTORY];
            if (r.object == object)
                return &r.trace;
        }
        return nullptr;
    }

    std::string shortName(const std::string &description)
    {
        std::string name = description.substr(0, description.find('('));
        if (name.size() > 60)
            name = "..." + name.substr(name.size() - 57);
        return name;
    }

} // namespace

namespace __cxxabiv1 {

    extern "C" void __cxa_throw(
        void *object, std::type_info *type, void (*destructor)(void *))
    {
        using Real = void (*)(void *, std::type_info *, void (*)(void *));
        static Real real =
            reinterpret_cast<Real>(dlsym(RTLD_NEXT, "__cxa_throw"));

        record(object);
        real(object, type, destructor);
        __builtin_unreachable();
    }

} // namespace __cxxabiv1

namespace Poc {

    std::string throwSite(const std::exception &exception)
    {
        const std::stacktrace *trace =
            find(dynamic_cast<const void *>(&exception));
        const std::string root = POC_SOURCE_ROOT;
        std::string result;

        if (trace == nullptr)
            return result;
        for (const auto &frame : *trace) {
            std::string file = frame.source_file();
            std::string name = frame.description();
            if (file.starts_with(root)) {
                file = file.substr(root.size());
                if (file.ends_with("Runner.cpp"))
                    break;
                result += "\n           " + file + ":" +
                    std::to_string(frame.source_line()) + "  " +
                    shortName(name);
            } else if (file.empty() &&
                name.find("RType::") != std::string::npos) {
                result += "\n           (no debug info)  " + shortName(name);
            }
        }
        if (!result.empty())
            result = "\n         thrown at:" + result;
        return result;
    }

} // namespace Poc

#else

namespace Poc {

    std::string throwSite(const std::exception &)
    {
        return "";
    }

} // namespace Poc

#endif
