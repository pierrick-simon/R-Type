#ifndef SOUND_HPP_
#define SOUND_HPP_

#include <memory>
#include <miniaudio.h>
#include <string>
#include "Decoder.hpp"
#include "Listener.hpp"
#include "Position2D.hpp"
#include "SoundInfo.hpp"

namespace RType {
    /**
     * @brief Spatial sound component, thin wrapper around a miniaudio ma_sound.
     *
     * Move-only: the underlying ma_sound lives on the heap, so it can be
     * moved into the component manager safely.
     *
     * The behaviour of the sound (state, looping, spatialization,
     * attenuation, distances) is driven by the fields inherited from
     * SoundInfo, which are applied every frame by systemSound().
     *
     * @warning The Listener that created the sound must outlive it.
     *
     * @see SoundInfo
     * @see Listener
     */
    class Sound : public SoundInfo {
    public:
        /**
         * @brief Load a sound from a file.
         *
         * The file is fully decoded in memory (MA_SOUND_FLAG_DECODE). The
         * looping, spatialization and attenuation settings are applied
         * from @p info right after loading.
         *
         * @param l    The Listener (audio engine) the sound belongs to.
         * @param path Path of the audio file to load.
         * @param info Initial settings of the sound.
         *
         * @throw EngineException If the sound cannot be loaded.
         */
        Sound(Listener &l, const std::string &path, SoundInfo info = {});
        Sound(Listener &l, Decoder &decoder, SoundInfo info = {});
        /**
         * @brief System applying the SoundInfo settings and handling the
         *        playback state of the sound.
         *
         * Updates looping, spatialization, attenuation model and distances
         * from the SoundInfo fields, then processes the current
         * SoundInfo::State (start, restart, pause, stop, end detection).
         *
         * @param s The sound to update.
         */
        static void systemSound(Sound &s);

        /**
         * @brief System updating the position of the sound in 2D space.
         *
         * Sets the position of the sound to (pos.x, pos.y, 0). Only has an
         * audible effect when spatialization is enabled (SoundInfo::stereo).
         *
         * @param s   The sound to move.
         * @param pos The new position of the sound.
         */
        static void systemSound2D(Sound &s, const Position2D &pos);

        /**
         * @brief Set the volume of the sound.
         *
         * @param volume Volume multiplier (1.0 is the original volume,
         *               0.0 is silence).
         */
        void setVolume(float volume);

        /**
         * @brief Set the pitch of the sound.
         *
         * @param pitch Pitch multiplier (1.0 is the original pitch).
         */
        void setPitch(float pitch);

        /**
         * @brief Get the underlying miniaudio sound.
         *
         * @return A reference to the @c ma_sound owned by this component.
         *
         * @warning The reference is only valid as long as the Sound is
         *          alive.
         */
        [[nodiscard]] ma_sound &getSound()
        {
            return *_sound;
        }

    private:
        /**
         * @brief Apply the playback action matching the current state.
         *
         * - PLAY: restart from the beginning, then switch to PLAYING.
         * - START: resume playback, then switch to PLAYING.
         * - PLAYING: switch to STOP once the end of the sound is reached.
         * - PAUSED: stop playback, keeping the cursor position.
         * - STOP: stop playback and rewind to the beginning.
         */
        void handleState();

        /**
         * @brief Custom deleter uninitialising the miniaudio sound.
         */
        struct SoundDeleter {
            /**
             * @brief Call @c ma_sound_uninit on the sound.
             *
             * @param s The sound to uninitialise.
             */
            void operator()(ma_sound *s) const;
        };

        std::unique_ptr<ma_sound, SoundDeleter> _sound;
    };
} // namespace RType

#endif /* !SOUND_HPP_ */
