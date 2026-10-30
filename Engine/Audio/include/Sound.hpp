#ifndef SOUND_HPP_
#define SOUND_HPP_

#include <memory>
#include <miniaudio.h>
#include <string>
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
     * @warning The Listener that created the sound must outlive it.
     */
    class Sound : public SoundInfo {
    public:
        /**
         * @throw EngineException If the sound cannot be loaded.
         */
        Sound(Listener &l, const std::string &path, SoundInfo info = {});

        static void systemSound(Sound &s);
        static void systemSound2D(Sound &s, const Position2D &pos);

        void setVolume(float volume);
        void setPitch(float pitch);

        [[nodiscard]] ma_sound &getSound()
        {
            return *_sound;
        }

    private:
        void handleState();

        struct SoundDeleter {
            void operator()(ma_sound *s) const;
        };

        std::unique_ptr<ma_sound, SoundDeleter> _sound;
    };
} // namespace RType

#endif /* !SOUND_HPP_ */
