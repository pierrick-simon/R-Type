#ifndef SOUND_HPP_
#define SOUND_HPP_

#include <memory>
#include <miniaudio.h>
#include <string>
#include "Listener.hpp"

namespace RType {
    /**
     * @brief Spatial sound component, thin wrapper around a miniaudio ma_sound.
     *
     * Move-only: the underlying ma_sound lives on the heap, so it can be
     * moved into the component manager safely.
     *
     * @warning The Listener that created the sound must outlive it.
     */
    class Sound {
    public:
        enum class State {
            PLAY, //!< Request to play the sound
            PLAYING, //!< The sound is currently playing
            PAUSED, //!< The sound is paused
            START, //!< The sound is about to start
            STOP, //!< The sound is stopped
        };

        /**
         * @throw EngineException If the sound cannot be loaded.
         */
        Sound(Listener &l, const std::string &path, bool loop = false);

        bool loop = false;
        State state = State::STOP;

        void Sound::systemSound(Sound &s);

        [[nodiscard]] ma_sound &getSound()
        {
            return *_sound;
        }

    private:
        std::unique_ptr<ma_sound> _sound;
    };
} // namespace RType

#endif /* !SOUND_HPP_ */
