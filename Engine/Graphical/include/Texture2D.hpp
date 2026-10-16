/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture2D
*/

#ifndef TEXTURE2D_HPP_
#define TEXTURE2D_HPP_

    #include <raylib-cpp.hpp>

namespace RType {

    /**
     * @brief Texture resource, thin wrapper around raylib::Texture2D.
     *
     * Used by Sprite2D and SpriteSheet2D systems.
     */
    struct Texture2D : raylib::Texture2D {
        /**
         * @brief Load a texture from an image file.
         *
         * @param path Path to the image file (e.g. .png).
         *
         * @throw EngineException If the texture cannot be loaded
         *        (missing file, unsupported format, ...).
         *
         * @warning The window (OpenGL context) must be created before loading
         *          a texture, otherwise loading will fail or crash.
         */
        Texture2D(const std::string &path);
    };

} // namespace RType
#endif /* !TEXTURE2D_HPP_ */
