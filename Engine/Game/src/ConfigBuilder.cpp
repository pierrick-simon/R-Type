/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ConfigBuilder
*/

#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "ConfigBuilder.hpp"
#include "Exceptions.hpp"

namespace RType::Game {

    const std::unordered_map<std::string, ConfigBuilder::Setter>
        ConfigBuilder::setters = {
        {"windowFPS",
            [](GameInfo &info, const std::string &value) {
                info.windowFPS = static_cast<int>(toPositive(value));
            }},
        {"windowWidth",
            [](GameInfo &info, const std::string &value) {
                info.windowWidth = toPositive(value);
            }},
        {"windowHeight",
            [](GameInfo &info, const std::string &value) {
                info.windowHeight = toPositive(value);
            }},
        {"windowName",
            [](GameInfo &info, const std::string &value) {
                info.windowName = value;
            }},
        {"modules",
            [](GameInfo &info, const std::string &value) {
                info.modules = toList(value);
            }},
        {"srcDirectory",
            [](GameInfo &info, const std::string &value) {
                info.srcDir = value;
            }},
    };

    void ConfigBuilder::build(GameInfo &info, const std::string &path)
    {
        std::ifstream file(path);
        std::string line;

        if (!file)
            return;
        for (std::size_t number = 1; std::getline(file, line); number++) {
            try {
                applyLine(info, trim(line));
            } catch (const std::invalid_argument &e) {
                throw ConfigError(
                    path + ":" + std::to_string(number), e.what());
            }
        }
    }

    void ConfigBuilder::applyLine(GameInfo &info, const std::string &line)
    {
        if (line.empty() || line.front() == '#')
            return;
        std::size_t equal = line.find('=');
        if (equal == std::string::npos)
            throw std::invalid_argument("expected 'key = value'");
        std::string key = trim(line.substr(0, equal));
        auto setter = setters.find(key);
        if (setter == setters.end())
            throw std::invalid_argument("unknown key '" + key + "'");
        try {
            setter->second(info, trim(line.substr(equal + 1)));
        } catch (const std::invalid_argument &e) {
            throw std::invalid_argument(key + " " + e.what());
        }
    }

    std::string ConfigBuilder::trim(const std::string &str)
    {
        std::string spaces = " \t\r";
        std::size_t begin = str.find_first_not_of(spaces);

        if (begin == std::string::npos)
            return "";
        return str.substr(begin, str.find_last_not_of(spaces) - begin + 1);
    }

    std::size_t ConfigBuilder::toPositive(const std::string &value)
    {
        std::size_t number = 0;
        std::string end = value.data() + value.size();
        auto [ptr, error] = std::from_chars(value.data(), end.data(), number);

        if (error != std::errc() || ptr != end || number == 0)
            throw std::invalid_argument(
                "expects a number greater than 0, got '" + value + "'");
        return number;
    }

    std::list<std::string> ConfigBuilder::toList(const std::string &value)
    {
        std::list<std::string> list;
        std::stringstream stream(value);
        std::string element;

        while (std::getline(stream, element, ',')) {
            element = trim(element);
            if (!element.empty())
                list.push_back(element);
        }
        return list;
    }

} // namespace RType::Game
