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
    Decoder::Decoder(const std::string &path) :
        _decoder(new ma_decoder)
    {
        ma_result result =
            ma_decoder_init_file(path.c_str(), nullptr, _decoder.get());
        if (result != MA_SUCCESS) {
            throw EngineException("Cannot load file " + path + ": " +
                ma_result_description(result));
        }
    }

    void Decoder::DecoderDeleter::operator()(ma_decoder *d) const
    {
        ma_decoder_uninit(d);
    }

    void Decoder::unload(GameManager &gm)
    {
        for (auto [e, _] : gm.view<Sound>().getVector())
            gm.destroyEntity(e);
    }
}; // namespace RType
