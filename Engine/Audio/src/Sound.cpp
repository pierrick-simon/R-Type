/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sound
*/

#include "Sound.hpp"
#include "Exceptions.hpp"

namespace RType {
    Sound::Sound(Listener &l, const std::string &path, bool loop) :
        loop(loop), _sound(new ma_sound())
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

        ma_sound_set_looping(_sound.get(), loop ? MA_TRUE : MA_FALSE);
        ma_sound_set_attenuation_model(
            _sound.get(), ma_attenuation_model_inverse);
        ma_sound_set_min_distance(_sound.get(), 1.0f);
        ma_sound_set_max_distance(_sound.get(), 50.0f);
    }

    void Sound::systemSound(Sound &s, SoundState &state)
    {
        if (!s._sound.get())
            return;

        ma_sound_set_looping(s._sound.get(), s.loop ? MA_TRUE : MA_FALSE);

        switch (state.state) {
            case SoundState::State::PLAY:
                ma_sound_seek_to_pcm_frame(s._sound.get(), 0);
                ma_sound_start(s._sound.get());
                state.state = SoundState::State::PLAYING;
                break;
            case SoundState::State::START:
                ma_sound_start(s._sound.get());
                state.state = SoundState::State::PLAYING;
                break;
            case SoundState::State::PLAYING:
                if (ma_sound_at_end(s._sound.get()))
                    state.state = SoundState::State::STOP;
                break;
            case SoundState::State::PAUSED:
                if (ma_sound_is_playing(s._sound.get()))
                    ma_sound_stop(s._sound.get());
                break;
            case SoundState::State::STOP: {
                if (ma_sound_is_playing(s._sound.get()))
                    ma_sound_stop(s._sound.get());
                ma_uint64 cursor = 0;
                ma_sound_get_cursor_in_pcm_frames(s._sound.get(), &cursor);
                if (cursor != 0)
                    ma_sound_seek_to_pcm_frame(s._sound.get(), 0);
                break;
            }
        }
    }

    void Sound::SoundDeleter::operator()(ma_sound *s) const
    {
        ma_sound_uninit(s);
    }
}; // namespace RType
