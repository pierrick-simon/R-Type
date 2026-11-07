/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Console
*/

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <istream>
#include <map>
#include <mutex>
#include <queue>
#include <sol/sol.hpp>
#include <string>

#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief Debug console: runs the commands typed in the terminal on the
     * game, through the functions already bound to Lua.
     *
     * listen reads the terminal in its own thread and only stores the
     * lines. run executes them on the game's thread, so the GameManager
     * and Lua are never used by two threads at once.
     */
    class Console {
    public:
        /**
         * @brief Unsyncs the C++ streams from stdio, so that a pending
         * read does not keep the game from exiting.
         */
        Console(sol::state &lua, GameManager &gm);

        /**
         * @brief Stores every line typed in the terminal, until exit or
         * exitConsole. Runs in its own thread.
         */
        static void listen();

        /**
         * @brief Executes the stored lines, then prints the prompt. Called
         * by the game's thread between two frames.
         */
        void run();

    private:
        typedef void (Console::*CommandFunction)(std::istream &args);

        struct Command {
            CommandFunction function = nullptr;
            std::string usage;
        };

        static const std::map<std::string, Command> _commands;
        static std::queue<std::string> _pending;
        static std::mutex _mutex;

        sol::state &_lua;
        GameManager &_gm;
        bool _prompt = true;
        bool _open = true;

        void execute(const std::string &line);

        void help(std::istream &args);
        void createEntity(std::istream &args);
        void destroyEntity(std::istream &args);
        void addComponent(std::istream &args);
        void removeComponent(std::istream &args);
        void enableSystem(std::istream &args);
        void disableSystem(std::istream &args);
        void exitGame(std::istream &args);
        void exitConsole(std::istream &args);

        /**
         * @brief Calls gm:function(args...) through its Lua binding.
         * @throw ScriptError If the call failed.
         */
        template<typename... Args>
        sol::protected_function_result call(
            const std::string &function, Args &&...args);

        /**
         * @throw ScriptError If the Lua call failed.
         */
        static void check(const sol::protected_function_result &result);

        /**
         * @throw std::invalid_argument If the next argument is missing or
         * is not a number.
         */
        static std::size_t readNumber(std::istream &args);

        /**
         * @throw std::invalid_argument If there is no argument left.
         */
        static std::string readWord(std::istream &args);
    };

    template<typename... Args>
    sol::protected_function_result Console::call(
        const std::string &function, Args &&...args)
    {
        sol::protected_function func = _lua["GameManager"][function];
        sol::protected_function_result result =
            func(&_gm, std::forward<Args>(args)...);

        check(result);
        return result;
    }

} // namespace RType::Game

#endif /* !CONSOLE_HPP_ */
