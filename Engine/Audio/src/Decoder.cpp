/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Decoder
*/

#include "Decoder.hpp"
#include "Exceptions.hpp"
#include "Sound.hpp"

namespace RType {
    Decoder::Decoder(const std::string &path)
    {
        ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_decoder dec;
        ma_result result = ma_decoder_init_file(path.c_str(), &cfg, &dec);
        if (result != MA_SUCCESS) {
            throw EngineException("Cannot load file " + path + ": " +
                ma_result_description(result));
        }

        ma_format fmt;
        ma_decoder_get_data_format(
            &dec, &fmt, &_channels, &_sampleRate, nullptr, 0);

        auto pcm = std::make_shared<std::vector<float>>();
        constexpr ma_uint64 CHUNK = 4096;
        ma_uint64 total = 0;
        for (;;) {
            pcm->resize((total + CHUNK) * _channels);
            ma_uint64 read = 0;
            ma_result r = ma_decoder_read_pcm_frames(
                &dec, pcm->data() + total * _channels, CHUNK, &read);
            total += read;
            if (r != MA_SUCCESS || read < CHUNK)
                break;
        }
        pcm->resize(total * _channels);
        ma_decoder_uninit(&dec);

        _pcm = std::move(pcm);
    }

    void Decoder::unload(GameManager &gm)
    {
        for (auto [e, _] : gm.view<Sound>().getVector())
            gm.destroyEntity(e);
    }
} // namespace RType
