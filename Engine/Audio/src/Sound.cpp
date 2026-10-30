/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sound
*/

#include "Sound.hpp"
#include "Exceptions.hpp"

namespace RType {
    Sound::Sound(Listener &l, const std::string &path, SoundInfo info) :
        SoundInfo(info), _sound(new ma_sound())
    {
        ma_result result = ma_sound_init_from_file(&l.getEngine(),
            path.c_str(),
            MA_SOUND_FLAG_DECODE,
            nullptr,
            nullptr,
            _sound.get());

        if (result != MA_SUCCESS) {
            throw EngineException("Cannot load sound " + path + ": " +
                ma_result_description(result));
        }

        if (stereo) {
            ma_sound_set_looping(_sound.get(), loop ? MA_TRUE : MA_FALSE);
            ma_sound_set_attenuation_model(
                _sound.get(), ma_attenuation_model(attenuation));
            ma_sound_set_min_distance(_sound.get(), minDistance);
            ma_sound_set_max_distance(_sound.get(), maxDistance);
        } else
            ma_sound_set_spatialization_enabled(_sound.get(), MA_FALSE);
    }

    void Sound::systemSound(Sound &s)
    {
        auto sound = s._sound.get();
        if (!sound)
            return;
        switch (s.state) {
            case SoundInfo::State::PLAY:
                ma_sound_seek_to_pcm_frame(sound, 0);
                ma_sound_start(sound);
                s.state = SoundInfo::State::PLAYING;
                break;
            case SoundInfo::State::START:
                ma_sound_start(sound);
                s.state = SoundInfo::State::PLAYING;
                break;
            case SoundInfo::State::PLAYING:
                if (ma_sound_at_end(sound))
                    s.state = SoundInfo::State::STOP;
                break;
            case SoundInfo::State::PAUSED:
                if (ma_sound_is_playing(sound))
                    ma_sound_stop(sound);
                break;
            case SoundInfo::State::STOP: {
                if (ma_sound_is_playing(sound))
                    ma_sound_stop(sound);
                ma_uint64 cursor = 0;
                ma_sound_get_cursor_in_pcm_frames(sound, &cursor);
                if (cursor != 0)
                    ma_sound_seek_to_pcm_frame(sound, 0);
                break;
            }
        }
    }

    void Sound::SoundDeleter::operator()(ma_sound *s) const
    {
        ma_sound_uninit(s);
    }
}; // namespace RType
