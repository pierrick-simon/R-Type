/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sound
*/

#ifndef SOUND_HPP_
#define SOUND_HPP_

    #include <raylib-cpp.hpp>

namespace RType {

    struct Sound : raylib::Sound {
        enum class State {
            PLAY,
            PLAYING,
            PAUSED,
            START,
            STOP,
        };

        Sound(const std::string &path, bool loop = false);

        bool loop;
        State state = State::STOP;

        static void systemSound(Sound &s);
    };

} // namespace RType

#endif /* !SOUND_HPP_ */
