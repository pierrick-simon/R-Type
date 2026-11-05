/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ConfigBuilder
*/

#ifndef CONFIGBUILDER_HPP_
#define CONFIGBUILDER_HPP_

#include <cstddef>
#include <functional>
#include <list>
#include <string>
#include <unordered_map>

#include "Game.hpp"

namespace RType::Game {

    /**
     * @brief Updates a GameInfo from a configuration file, so that the
     * window and the modules can be changed without recompiling the game.
     *
     * The file holds one "key = value" per line. Every key is optional: a
     * key left out keeps the value given by the game. Empty lines and lines
     * starting with '#' are ignored.
     *
     * windowFPS = 60
     * windowWidth = 1280
     * windowHeight = 720
     * windowName = My Game
     * modules = Window, Keyboard, Mouse
     */
    class ConfigBuilder {
    public:
        ConfigBuilder() = delete;

        /**
         * @brief Applies every line of the file to the GameInfo. Does
         * nothing if the file does not exist or cannot be opened.
         * @param info The GameInfo to update, usually GameConfig.
         * @param path The path of the configuration file.
         * @throw ConfigError If a line has no '=', an unknown key or an
         * invalid value, with the file and the line.
         */
        static void build(GameInfo &info, const std::string &path);

    private:
        /**
         * @brief Applies one line, already trimmed, to the GameInfo. Does
         * nothing for an empty line or a comment.
         * @throw std::invalid_argument If the line has no '=', an unknown
         * key or an invalid value, with the reason as its message.
         */
        static void applyLine(GameInfo &info, const std::string &line);

        /**
         * @brief Applies the value of one key to the GameInfo.
         * @throw std::invalid_argument If the value is invalid, with the
         * reason as its message.
         */
        using Setter =
            std::function<void(GameInfo &, const std::string &value)>;

        /**
         * @brief Every key the file can hold, with how its value is
         * applied. Adding a setting to the file is adding one entry here.
         */
        static const std::unordered_map<std::string, Setter> setters;

        /**
         * @brief Removes the spaces and tabs at both ends of a string.
         */
        static std::string trim(const std::string &str);

        /**
         * @brief Reads a whole positive number, such as "60".
         * @throw std::invalid_argument If the value is not a number greater
         * than 0.
         */
        static std::size_t toPositive(const std::string &value);

        /**
         * @brief Reads a list separated by commas, such as
         * "Window, Keyboard", without the spaces around each element.
         * Empty elements are left out.
         */
        static std::list<std::string> toList(const std::string &value);
    };

} // namespace RType::Game

#endif /* !CONFIGBUILDER_HPP_ */
