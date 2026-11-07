/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Console
*/

#ifndef CONSOLE_HPP_
#define CONSOLE_HPP_

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <sol/sol.hpp>
#include <sstream>
#include <string>
#include <unordered_map>

#include "GameManager.hpp"

namespace RType {

    class Console {
        public:
            static void listen(const std::atomic<bool> &running);
            static void run(sol::state &lua, GameManager &gm);

        private:
            using Command = std::function<void(sol::state &, GameManager &, const std::string &)>;

            static void help(sol::state &lua, GameManager &gm, const std::string &args);
            static void createEntity(sol::state &lua, GameManager &gm, const std::string &args);
            static void destroyEntity(sol::state &lua, GameManager &gm, const std::string &args);
            static void addComponent(sol::state &lua, GameManager &gm, const std::string &args);
            static void removeComponent(sol::state &lua, GameManager &gm, const std::string &args);
            static void enableSystem(sol::state &lua, GameManager &gm, const std::string &args);
            static void disableSystem(sol::state &lua, GameManager &gm, const std::string &args);
            static void exitConsole(sol::state &lua, GameManager &gm, const std::string &args);
            static void exitGame(sol::state &lua, GameManager &gm, const std::string &args);

            static void execute(sol::state &lua, GameManager &gm, const std::string &input);
            static Entity readEntity(std::istringstream &stream);

            template<typename... Args>
            static sol::protected_function_result call(
                sol::state &lua, const std::string &function, GameManager &gm, Args &&...args);

            static const std::unordered_map<std::string, Command> _commands;
            static std::queue<std::string> _pending;
            static std::mutex _mutex;
            static bool _needPrompt;
            static bool _started;
            static bool _closed;
    };

} // namespace RType

#endif /* !CONSOLE_HPP_ */
