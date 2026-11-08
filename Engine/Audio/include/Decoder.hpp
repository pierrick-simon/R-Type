/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Decoder
*/

#ifndef DECODER_HPP_
#define DECODER_HPP_

#include <memory>
#include <miniaudio.h>
#include <string>
#include <string_view>
#include <vector>
#include "GameManager.hpp"

namespace RType {
    class Decoder {
    public:
        static constexpr std::string_view TYPE = "sound";

        /**
         * @param path Audio file to decode fully in memory (float32).
         * @param sampleRate Target sample rate. Pass the engine's rate
         * (ma_engine_get_sample_rate) so no resampling
         * is needed. 0 keeps the file's native rate.
         */
        Decoder(const std::string &path);

        static void unload(GameManager &gm);

        using Pcm = std::shared_ptr<const std::vector<float>>;

        [[nodiscard]] Pcm getPcm() const
        {
            return _pcm;
        }

        [[nodiscard]] ma_uint32 getChannels() const
        {
            return _channels;
        }

        [[nodiscard]] ma_uint32 getSampleRate() const
        {
            return _sampleRate;
        }

        [[nodiscard]] ma_uint64 getFrameCount() const
        {
            return _channels ? _pcm->size() / _channels : 0;
        }

    private:
        Pcm _pcm;
        ma_uint32 _channels = 0;
        ma_uint32 _sampleRate = 0;
    };
} // namespace RType

#endif /* !DECODER_HPP_ */
