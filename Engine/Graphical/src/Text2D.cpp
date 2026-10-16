/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2D
*/

#include "Text2D.hpp"

namespace RType {
    void Text2D::systemDrawText2D(
<<<<<<< HEAD
        Text2D &text, const Font &font, const Position2D &pos)
=======
        const Text2D &text, const Font &font, const Position2D &pos)
>>>>>>> dev
    {
        ::DrawTextPro(font,
            text.str.c_str(),
            {pos.x, pos.y},
            {text.origin.x, text.origin.y},
            text.rotation,
            text.fontSize,
            text.spacing,
            text.color);
    }
} // namespace RType
