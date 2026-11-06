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
#include "GameManager.hpp"

namespace RType {
    /**
     * @brief Audio decoder component, thin wrapper around a miniaudio
     *        ma_decoder.
     *
     * Owns a decoder initialised from an audio file, so the same decoded
     * source can be shared by one or several Sound components.
     *
     * Move-only: the underlying ma_decoder lives on the heap, so it can be
     * moved into the component manager safely. It is automatically
     * uninitialised when the Decoder is destroyed.
     *
     * @warning Every Sound created from this Decoder must be destroyed
     *          before it. See unload().
     *
     * @see Sound
     */
    class Decoder {
    public:
        /**
         * @brief Create a decoder and initialise it.
         *
         * @param path Path of the audio file to load.
         *
         * @throw EngineException If the sound cannot be loaded.
         */
        Decoder(const std::string &path);

        /**
         * @brief Unload every entity linked to a decoder.
         *
         * Releases all the entities that use a Decoder, so the underlying
         * ma_decoder can be safely uninitialised afterwards.
         *
         * @param gm The GameManager holding the entities and components.
         */
        static void unload(GameManager &gm);

        /**
         * @brief Get the underlying miniaudio decoder.
         *
         * @return A reference to the @c ma_decoder owned by this component.
         *
         * @warning The reference is only valid as long as the Decoder is
         *          alive.
         */
        [[nodiscard]] ma_decoder &getDecoder()
        {
            return *_decoder;
        }

    private:
        /**
         * @brief Custom deleter uninitialising the miniaudio decoder.
         */
        struct DecoderDeleter {
            /**
             * @brief Call @c ma_decoder_uninit on the decoder.
             *
             * @param s The decoder to uninitialise.
             */
            void operator()(ma_decoder *s) const;
        };

        std::unique_ptr<ma_decoder, DecoderDeleter> _decoder;
    };
} // namespace RType

#endif /* !DECODER_HPP_ */
