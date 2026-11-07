/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Console
*/

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Console.hpp"
#include "Exceptions.hpp"

namespace RType::Game {

    const std::map<std::string, Console::Command> Console::_commands = {
        {"help", {&Console::help, ""}},
        {"createEntity", {&Console::createEntity, ""}},
        {"destroyEntity", {&Console::destroyEntity, "<index> <generation>"}},
        {"addComponent",
            {&Console::addComponent,
                "<index> <generation> <component> [{fields}]"}},
        {"removeComponent",
            {&Console::removeComponent, "<index> <generation> <component>"}},
        {"enableSystem", {&Console::enableSystem, "<system>"}},
        {"disableSystem", {&Console::disableSystem, "<id>"}},
        {"exit", {&Console::exitGame, ""}},
        {"exitConsole", {&Console::exitConsole, ""}},
    };

    std::queue<std::string> Console::_pending;
    std::mutex Console::_mutex;

    Console::Console(sol::state &lua, GameManager &gm) :
        _lua(lua), _gm(gm)
    {
        std::ios::sync_with_stdio(false);
    }

    void Console::listen()
    {
        std::string line;
        std::string name;

        while (std::getline(std::cin, line)) {
            _mutex.lock();
            _pending.push(line);
            _mutex.unlock();
            name = line.substr(0, line.find(' '));
            if (name == "exit" || name == "exitConsole")
                return;
        }
    }

    void Console::run()
    {
        std::queue<std::string> lines;

        _mutex.lock();
        std::swap(lines, _pending);
        _mutex.unlock();
        while (!lines.empty()) {
            execute(lines.front());
            lines.pop();
            _prompt = true;
        }
        if (_prompt && _open) {
            std::cout << "> " << std::flush;
            _prompt = false;
        }
    }

    void Console::execute(const std::string &line)
    {
        std::istringstream args(line);
        std::string name;

        args >> name;
        if (name.empty())
            return;
        if (!_commands.contains(name)) {
            std::cerr << "Unknown command: " << name << std::endl;
            return;
        }
        const Command &command = _commands.at(name);

        try {
            (this->*command.function)(args);
        } catch (const std::invalid_argument &) {
            std::cerr << "usage: " << name << " " << command.usage << std::endl;
        } catch (const std::exception &e) {
            std::cerr << name << ": " << e.what() << std::endl;
        }
    }

    void Console::help(std::istream &)
    {
        for (const auto &[name, command] : _commands)
            std::cout << name << " " << command.usage << std::endl;
    }

    void Console::createEntity(std::istream &)
    {
        Entity entity = this->call("createEntity").get<Entity>();

        std::cout << "#" << entity.index() << " (" << entity.generation() << ")"
                  << std::endl;
    }

    void Console::destroyEntity(std::istream &args)
    {
        std::size_t index = readNumber(args);
        std::size_t generation = readNumber(args);

        if (index == 0 && generation == 0)
            throw std::runtime_error("cannot destroy the window");
        this->call("destroyEntity", Entity(index, generation));
    }

    void Console::addComponent(std::istream &args)
    {
        std::size_t index = readNumber(args);
        std::size_t generation = readNumber(args);
        std::string name = readWord(args);
        std::string fields;

        args >> std::ws;
        std::getline(args, fields);
        if (fields.empty()) {
            this->call("addComponent", Entity(index, generation), name);
            return;
        }
        sol::protected_function_result table =
            _lua.safe_script("return " + fields, &sol::script_pass_on_error);

        check(table);
        if (table.get_type() != sol::type::table)
            throw std::runtime_error(
                "fields must be a table, such as {x = 10}");
        this->call("addComponent",
            Entity(index, generation),
            name,
            table.get<sol::table>());
    }

    void Console::removeComponent(std::istream &args)
    {
        std::size_t index = readNumber(args);
        std::size_t generation = readNumber(args);
        std::string name = readWord(args);

        this->call("removeComponent", Entity(index, generation), name);
    }

    void Console::enableSystem(std::istream &args)
    {
        std::string name = readWord(args);
        SystemId id = this->call("addSystem", name).get<SystemId>();

        std::cout << "system " << id << std::endl;
    }

    void Console::disableSystem(std::istream &args)
    {
        SystemId id = readNumber(args);

        this->call("destroySystem", id);
    }

    void Console::exitGame(std::istream &)
    {
        this->call("stop");
        _open = false;
    }

    void Console::exitConsole(std::istream &)
    {
        _open = false;
    }

    void Console::check(const sol::protected_function_result &result)
    {
        if (result.valid())
            return;
        sol::error error = result;
        std::string message = error.what();

        throw ScriptError(message.substr(0, message.find("\nstack traceback")));
    }

    std::size_t Console::readNumber(std::istream &args)
    {
        std::size_t number = 0;

        if (!(args >> number))
            throw std::invalid_argument("expected a number");
        return number;
    }

    std::string Console::readWord(std::istream &args)
    {
        std::string word;

        if (!(args >> word))
            throw std::invalid_argument("expected a word");
        return word;
    }

} // namespace RType::Game
