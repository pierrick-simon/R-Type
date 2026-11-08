/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AudioBinding
*/

#ifndef AUDIOBINDING_HPP_
#define AUDIOBINDING_HPP_

#include <sol/sol.hpp>

namespace RType::Game {

    /**
     * @brief Lets scripts move the point that hears the sounds of the Audio
     * module, without exposing the Listener itself.
     */
    class AudioBinding {
    public:
        AudioBinding() = delete;

        /**
         * @brief Lets scripts call gm:setListenerPosition(x, y), which puts
         * the point that hears the stereo sounds at (x, y).
         * @throw ModuleNotLoaded If gm:setListenerPosition is called without
         * the Audio module.
         */
        static void registerFunctions(sol::state &lua);
    };

} // namespace RType::Game

#endif /* !AUDIOBINDING_HPP_ */
