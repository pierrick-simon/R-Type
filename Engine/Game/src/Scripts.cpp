/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Scripts
*/

#include <filesystem>

#include "Exceptions.hpp"
#include "Scripts.hpp"

namespace RType::Game {

    std::list<std::string> Scripts::findPaths(const std::string &directory)
    {
        std::list<std::string> paths;

        if (!std::filesystem::is_directory(directory))
            return paths;
        for (auto &entry :
            std::filesystem::recursive_directory_iterator(directory,
                std::filesystem::directory_options::skip_permission_denied))
            if (entry.is_regular_file() && entry.path().extension() == ".lua")
                paths.push_back(entry.path().string());
        paths.sort();
        return paths;
    }

    void Scripts::load(
        sol::state &lua, const std::string &path, GameManager &gm)
    {
        sol::environment env(lua, sol::create, lua.globals());

        checkResult(lua.script_file(path, env, &sol::script_pass_on_error));
        checkResult(env["start"](gm));
    }

    void Scripts::checkResult(const sol::protected_function_result &result)
    {
        if (!result.valid()) {
            sol::error error = result;
            throw ScriptError(error.what());
        }
    }

    int Scripts::handleException(lua_State *L,
        sol::optional<const std::exception &> exception,
        sol::string_view description)
    {
        const auto *engineError = exception
            ? dynamic_cast<const EngineException *>(&exception.value())
            : nullptr;

        if (engineError)
            return sol::stack::push(L, engineError->message());
        return sol::stack::push(L, description);
    }

} // namespace RType::Game
