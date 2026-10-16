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

    /**
     * @brief Text component drawn on screen at a given position.
     */
    struct Text2D {
        float fontSize = 10;
        std::string str = "Hello World";
        raylib::Color color = raylib::Color::White();
        float rotation = 0;
        float spacing = 0;
        Origin2D origin;

        /**
         * @brief System drawing a text.
         *
         * @param text The text component to draw.
         * @param font The font used to render the text.
         * @param pos  The position where the text is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning @p font must stay valid for the whole call.
         */
        static void systemDrawText2D(
            Text2D &text, const Font &font, const Position2D &pos);
    };

} // namespace RType

#endif /* !TEXT2D_HPP_ */
