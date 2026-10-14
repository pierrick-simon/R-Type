/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sound
*/

#include "Sound.hpp"
#include "Exception.hpp"

namespace RType {
    Sound::Sound(const std::string &path, bool loop)
    {
        this->loop = loop;
        try {
            this->Load(path);
        } catch (raylib::RaylibException &e) {
            throw EngineException(e.what());
        }
    }

    void Sound::systemSound(Sound &s)
    {
        if (s.IsPlaying()) {
            if (s.state == State::STOP)
                s.Stop();
            if (s.state == State::PAUSED)
                s.Pause();
            if (s.state == State::PLAY) {
                s.Play();
                s.state = State::PLAYING;
            }
        } else {
            if (s.state == State::PLAYING && s.loop)
                s.Play();
            if (s.state == State::PLAYING && !s.loop)
                s.state = State::STOP;
            if (s.state == State::START) {
                s.Resume();
                s.state = State::PLAYING;
            }
            if (s.state == State::PLAY) {
                s.Play();
                s.state = State::PLAYING;
            }
        }
    }
}; // namespace RType
