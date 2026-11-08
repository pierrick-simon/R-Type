/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Scripts
*/

#ifndef SCRIPTS_HPP_
#define SCRIPTS_HPP_

#include <exception>
#include <list>
#include <sol/sol.hpp>
#include <string>

#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Finds and runs the game's Lua scripts, and turns their errors
     * into engine errors.
     */
    class Scripts {
    public:
        Scripts() = delete;

        /**
         * @brief Lists every Lua script inside a directory, including its
         * subdirectories.
         * @param directory The directory to search in.
         * @return The path of every .lua file, sorted alphabetically. Empty
         * if the directory does not exist.
         */
        static std::list<std::string> findPaths(const std::string &directory);

        /**
         * @brief Runs a Lua script in its own environment, then calls its
         * start function with the GameManager.
         * @param path The path of the .lua file to load.
         */
        static void load(
            sol::state &lua, const std::string &path, GameManager &gm);

        /**
         * @brief Turns a failed Lua call into an engine error, so that it
         * stops the game instead of being silently ignored.
         * @throw ScriptError If the call raised a Lua error.
         */
        static void checkResult(const sol::protected_function_result &result);

        /**
         * @brief Turns a C++ exception thrown while Lua was calling the
         * engine into a Lua error. An EngineException is passed without its
         * "Engine Error: " prefix, which checkResult adds back once.
         */
        static int handleException(lua_State *L,
            sol::optional<const std::exception &> exception,
            sol::string_view description);
    };

} // namespace RType::Game

#endif /* !SCRIPTS_HPP_ */
