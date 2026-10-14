/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sound
*/

#include "Sound.hpp"

namespace RType {
    Sound::Sound(const std::string &path, bool loop)
    {
        this->loop = loop;
        _sound.Load(path);
    }

    void Sound::systemSound(Sound &s)
    {
        if (s._sound.IsPlaying()) {
            if (s.state == State::STOP)
                s._sound.Stop();
            if (s.state == State::PAUSED)
                s._sound.Pause();
            if (s.state == State::PLAY) {
                s._sound.Play();
                s.state = State::PLAYING;
            }
        } else {
            if (s.state == State::PLAYING && s.loop)
                s._sound.Play();
            if (s.state == State::PLAYING && !s.loop)
                s.state = State::STOP;
            if (s.state == State::START) {
                s._sound.Resume();
                s.state = State::PLAYING;
            }
            if (s.state == State::PLAY) {
                s._sound.Play();
                s.state = State::PLAYING;
            }
        }
    }
}; // namespace RType
