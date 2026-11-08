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
        systemSound(*this);
    }

    Sound::Sound(Listener &l, Decoder &decoder, SoundInfo info) :
        SoundInfo(info),
        _pcm(decoder.getPcm()),
        _ref(new ma_audio_buffer_ref()),
        _sound(new ma_sound())
    {
        ma_result r = ma_audio_buffer_ref_init(ma_format_f32,
            decoder.getChannels(),
            _pcm->data(),
            decoder.getFrameCount(),
            _ref.get());
        if (r != MA_SUCCESS)
            throw EngineException("Failed to create audio buffer ref");

        r = ma_sound_init_from_data_source(
            &l.getEngine(), _ref.get(), 0, nullptr, _sound.get());
        if (r != MA_SUCCESS)
            throw EngineException("Failed to initialise sound from decoder");

        systemSound(*this);
    }

    void Sound::handleState()
    {
        auto sound = _sound.get();
        switch (state) {
            case SoundInfo::State::PLAY:
                ma_sound_seek_to_pcm_frame(sound, 0);
                ma_sound_start(sound);
                state = SoundInfo::State::PLAYING;
                break;
            case SoundInfo::State::START:
                ma_sound_start(sound);
                state = SoundInfo::State::PLAYING;
                break;
            case SoundInfo::State::PLAYING:
                if (ma_sound_at_end(sound))
                    state = SoundInfo::State::STOP;
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

    void Sound::systemSound(Sound &s)
    {
        auto sound = s._sound.get();
        if (!sound)
            return;
        ma_sound_set_looping(sound, s.loop ? MA_TRUE : MA_FALSE);
        if (s.stereo) {
            ma_sound_set_attenuation_model(
                sound, ma_attenuation_model(s.attenuation));
            ma_sound_set_min_distance(sound, s.minDistance);
            ma_sound_set_max_distance(sound, s.maxDistance);
            ma_sound_set_spatialization_enabled(sound, MA_TRUE);
        } else
            ma_sound_set_spatialization_enabled(sound, MA_FALSE);
        s.handleState();
    }

    void Sound::systemSound2D(Sound &s, const Position2D &pos)
    {
        ma_sound_set_position(s._sound.get(), pos.x, pos.y, 0.f);
    }

    void Sound::setVolume(float volume)
    {
        ma_sound_set_volume(_sound.get(), volume);
    }

    void Sound::setPitch(float pitch)
    {
        ma_sound_set_pitch(_sound.get(), pitch);
    }

    void Sound::SoundDeleter::operator()(ma_sound *s) const
    {
        ma_sound_uninit(s);
    }
}; // namespace RType
