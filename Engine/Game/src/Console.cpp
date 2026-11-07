/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Console
*/

#include "Console.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>

#include "Scripts.hpp"

namespace RType {

    const std::unordered_map<std::string, Console::Command> Console::_commands = {
        {"help", &Console::help},
        {"createEntity", &Console::createEntity},
        {"destroyEntity", &Console::destroyEntity},
        {"addComponent", &Console::addComponent},
        {"removeComponent", &Console::removeComponent},
        {"enableSystem", &Console::enableSystem},
        {"disableSystem", &Console::disableSystem},
        {"exit", &Console::exitGame},
        {"exitConsole", &Console::exitConsole},
    };

    std::queue<std::string> Console::_pending;
    std::mutex Console::_mutex;
    bool Console::_needPrompt = true;
    bool Console::_started = false;
    bool Console::_closed = false;

    template<typename... Args>
    sol::protected_function_result Console::call(
        sol::state &lua, const std::string &function, GameManager &gm, Args &&...args)
    {
        sol::protected_function func = lua["GameManager"][function];
        sol::protected_function_result result = func(&gm, std::forward<Args>(args)...);

        if (!result.valid()) {
            sol::error error = result;
            std::string message = error.what();

            throw ScriptError(message.substr(0, message.find("\nstack traceback:")));
        }
        return result;
    }

    Entity Console::readEntity(std::istringstream &stream)
    {
        std::size_t index = 0;
        std::size_t generation = 0;

        if (!(stream >> index >> generation))
            throw std::invalid_argument("expected <index> <generation>");
        return Entity(index, generation);
    }

    void Console::help(sol::state &, GameManager &, const std::string &)
    {
        for (const auto &[name, _] : _commands)
            std::cout << name << std::endl;
    }

    void Console::createEntity(sol::state &lua, GameManager &gm, const std::string &)
    {
        Entity entity = call(lua, "createEntity", gm).get<Entity>();

        std::cout << "#" << entity.index() << " (" << entity.generation() << ")" << std::endl;
    }

    void Console::destroyEntity(sol::state &lua, GameManager &gm, const std::string &args)
    {
        std::istringstream stream(args);
        Entity entity = readEntity(stream);

        if (entity.index() == 0 && entity.generation() == 0)
            throw std::invalid_argument("cannot destroy the window");
        call(lua, "destroyEntity", gm, entity);
    }

    void Console::addComponent(sol::state &lua, GameManager &gm, const std::string &args)
    {
        std::istringstream stream(args);
        Entity entity = readEntity(stream);
        std::string name;
        std::string fields;

        if (!(stream >> name))
            throw std::invalid_argument("expected <index> <generation> <component> [{fields}]");
        std::getline(stream >> std::ws, fields);
        if (fields.empty()) {
            call(lua, "addComponent", gm, entity, name);
            return;
        }
        sol::protected_function_result table = lua.safe_script("return " + fields, &sol::script_pass_on_error);
        Game::Scripts::checkResult(table);
        if (table.get_type() != sol::type::table)
            throw std::invalid_argument("fields must be a Lua table, such as {x = 10}");
        call(lua, "addComponent", gm, entity, name, table.get<sol::table>());
    }

    void Console::removeComponent(sol::state &lua, GameManager &gm, const std::string &args)
    {
        std::istringstream stream(args);
        Entity entity = readEntity(stream);
        std::string name;

        if (!(stream >> name))
            throw std::invalid_argument("expected <index> <generation> <component>");
        call(lua, "removeComponent", gm, entity, name);
    }

    void Console::enableSystem(sol::state &lua, GameManager &gm, const std::string &args)
    {
        std::istringstream stream(args);
        std::string name;

        if (!(stream >> name))
            throw std::invalid_argument("expected <system>");
        SystemId id = call(lua, "addSystem", gm, name).get<SystemId>();

        std::cout << "system " << id << std::endl;
    }

    void Console::disableSystem(sol::state &lua, GameManager &gm, const std::string &args)
    {
        std::istringstream stream(args);
        SystemId id = 0;

        if (!(stream >> id))
            throw std::invalid_argument("expected <id>");
        call(lua, "destroySystem", gm, id);
    }

    void Console::exitConsole(sol::state &, GameManager &, const std::string &)
    {
        _closed = true;
    }

    void Console::exitGame(sol::state &lua, GameManager &gm, const std::string &)
    {
        call(lua, "stop", gm);
        _closed = true;
    }

    void Console::listen(const std::atomic<bool> &running)
    {
        std::string input;

        while (running && std::getline(std::cin, input)) {
            const std::string name = input.substr(0, input.find(' '));

            {
                std::lock_guard lock(_mutex);
                _pending.push(input);
            }
            if (name == "exit" || name == "exitConsole")
                return;
        }
    }

    void Console::run(sol::state &lua, GameManager &gm)
    {
        std::queue<std::string> pending;

        {
            std::lock_guard lock(_mutex);
            std::swap(pending, _pending);
        }
        if (!pending.empty())
            _needPrompt = true;
        for (; !pending.empty(); pending.pop())
            execute(lua, gm, pending.front());
        if (_needPrompt && _started && !_closed) {
            std::cout << "> " << std::flush;
            _needPrompt = false;
        }
        _started = true;
    }

    void Console::execute(sol::state &lua, GameManager &gm, const std::string &input)
    {
        const std::size_t space = input.find(' ');
        const std::string name = input.substr(0, space);
        const std::string args = space == std::string::npos ? "" : input.substr(space + 1);

        auto it = _commands.find(name);
        if (it == _commands.end()) {
            std::cerr << "Unknown command: " << name << std::endl;
            return;
        }
        try {
            it->second(lua, gm, args);
        } catch (const std::exception &e) {
            std::cerr << name << ": " << e.what() << std::endl;
        } catch (...) {
            std::cerr << name << ": unknown error" << std::endl;
        }
    }

} // namespace RType
