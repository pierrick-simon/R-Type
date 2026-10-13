/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2d
*/

#ifndef TEXT2D_HPP_
#define TEXT2D_HPP_

#include <raylib-cpp.hpp>
#include "Font.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"

namespace RType {

    struct Text2D {
        Font &font;
        Position2D &pos;
        float fontSize = 10;
        std::string str = "Hello World";
        raylib::Color color = raylib::Color::White();
        float rotation = 0;
        float spacing = 0;
        Origin2D origin;

        static void systemText2D(Text2D &t);
    };

} // namespace RType

#endif /* !TEXT2D_HPP_ */
