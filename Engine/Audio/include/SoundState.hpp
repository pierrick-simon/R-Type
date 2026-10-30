/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SoundState
*/

#ifndef SOUNDSTATE_HPP_
#define SOUNDSTATE_HPP_

namespace RType {
    struct SoundState {
        enum class State {
            PLAY, //!< Request to play the sound
            PLAYING, //!< The sound is currently playing
            PAUSED, //!< The sound is paused
            START, //!< The sound is about to start
            STOP, //!< The sound is stopped
        };

        State state = State::STOP;
    };
} // namespace RType

#endif /* !SOUNDSTATE_HPP_ */
