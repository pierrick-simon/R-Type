/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2D
*/

#include "Text2D.hpp"

namespace RType {
    void Text2D::systemText2D(Text2D &t)
    {
        ::DrawTextPro(t.font.font,
            t.str.c_str(),
            {t.pos.x, t.pos.y},
            {t.origin.x, t.origin.y},
            t.rotation,
            t.fontSize,
            t.spacing,
            t.color);
    }
} // namespace RType
