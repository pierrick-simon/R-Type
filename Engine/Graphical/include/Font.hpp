/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Font
*/

#ifndef FONT_HPP_
#define FONT_HPP_

    #include <raylib-cpp.hpp>

namespace RType {

    /**
     * @brief Font resource, thin wrapper around raylib::Font.
     *
     * Used by Text2D::systemText2D to render text.
     */
    struct Font : public raylib::Font {
        /**
         * @brief Load a font from a file.
         *
         * @param path Path to the font file (e.g. a .ttf file).
         *
         * @throw raylib::RaylibException If the font cannot be loaded
         *        (missing file, unsupported format, ...).
         *
         * @warning The window (OpenGL context) must be created before loading
         *          a font, otherwise loading will fail or crash.
         */
        Font(const std::string &path);
    };

} // namespace RType
#endif /* !FONT_HPP_ */
