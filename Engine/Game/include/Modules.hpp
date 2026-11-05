/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Modules
*/

#ifndef MODULES_HPP_
#define MODULES_HPP_

#include <functional>
#include <list>
#include <string>
#include <unordered_map>

#include "Game.hpp"
#include "GameManager.hpp"

namespace RType::Game {

    /**
     * @brief An engine feature the game can ask for in GameInfo::modules.
     */
    struct Module {
        std::list<std::string> dependencies;
        std::function<void(GameManager &, const GameInfo &)> create;
    };

    /**
     * @brief The modules the engine provides, and how the ones asked by
     * the game are checked and created.
     */
    class Modules {
    public:
        Modules() = delete;

        /**
         * @brief Merges the modules asked by the game with the default
         * ones, without duplicates, keeping the game's order first.
         */
        static std::list<std::string> resolve();

        /**
         * @brief Checks that every module exists and that its dependencies
         * are loaded too.
         * @throw NoSuchModule If a module is not in allowed.
         * @throw MissingModule If a dependency of a module is missing.
         */
        static void check(const std::list<std::string> &modules);

        /**
         * @brief Creates every module, in order.
         */
        static void create(
            GameManager &gm, const std::list<std::string> &modules);

    private:
        /**
         * @brief Every module the game can ask for, by name.
         */
        static const std::unordered_map<std::string, Module> allowed;

        /**
         * @brief The modules loaded even if the game does not ask for
         * them.
         */
        static const std::list<std::string> defaults;
    };

} // namespace RType::Game

#endif /* !MODULES_HPP_ */
