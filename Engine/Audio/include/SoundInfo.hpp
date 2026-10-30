/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SoundInfo
*/

#ifndef SOUNDINFO_HPP_
#define SOUNDINFO_HPP_

#include <cfloat>
#include <miniaudio.h>

namespace RType {
    struct SoundInfo {
        enum class State {
            PLAY, //!< Request to play the sound
            PLAYING, //!< The sound is currently playing
            PAUSED, //!< The sound is paused
            START, //!< The sound is about to start
            STOP, //!< The sound is stopped
        };

        enum Attenuation {
            NONE = ma_attenuation_model_none, //!< No distance attenuation and
                                              //!< no spatialization.
            INVERSE =
                ma_attenuation_model_inverse, //!< Equivalent to OpenAL's
                                              //!< AL_INVERSE_DISTANCE_CLAMPED.
            LINEAR =
                ma_attenuation_model_linear, //!< Linear attenuation. Equivalent
                                             //!< to OpenAL's
                                             //!< AL_LINEAR_DISTANCE_CLAMPED.
            EXPONENTIAL =
                ma_attenuation_model_exponential //!< Exponential attenuation.
                                                 //!< Equivalent to OpenAL's
                                                 //!< AL_EXPONENT_DISTANCE_CLAMPED.
        };

        bool loop = false;
        State state = State::STOP;
        bool stereo = false;
        Attenuation attenuation = INVERSE;
        float minDistance = 1.0f;
        float maxDistance = FLT_MAX;
    };
} // namespace RType

#endif /* !SOUNDINFO_HPP_ */
