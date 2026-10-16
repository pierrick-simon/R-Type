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

    /**
     * @brief Sound component, thin wrapper around raylib::Sound.
     *
     * Playback is driven by changing #state and calling systemSound().
     */
    struct Sound : raylib::Sound {
        /**
         * @brief Playback state of a sound.
         */
        enum class State {
            PLAY, //!< Request to play the sound
            PLAYING, //!< The sound is currently playing
            PAUSED, //!< The sound is paused
            START, //!< The sound is about to start
            STOP, //!< The sound is stopped
        };

        /**
         * @brief Load a sound from a file.
         *
         * @param path Path to the audio file (e.g. .wav, .ogg).
         * @param loop Whether the sound restarts automatically once finished.
         *
         * @throw EngineException If the sound cannot be loaded
         *        (missing file, unsupported format, ...).
         *
         * @warning The audio device must be initialized beforehand (it is
         *          owned by RType::Window), otherwise nothing will be heard.
         */
        Sound(const std::string &path, bool loop = false);

        bool loop;
        State state = State::STOP;

        /**
         * @brief System updating a sound according to its #state.
         *
         * @param s The sound to update.
         *
         * @warning Must be called once per frame, after the state has been
         *          modified, for the change to be taken into account.
         */
        static void systemSound(Sound &s);
    };

} // namespace RType

#endif /* !SOUND_HPP_ */
