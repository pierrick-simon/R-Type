/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#ifndef SPRITE2D_HPP_
#define SPRITE2D_HPP_

#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing how a whole texture is drawn.
     */
    struct Sprite2D {
        Origin2D origin;
        float scale = 1;
        float rotation = 0;
        bool flip = false;
        raylib::Color color = raylib::Color::White();

        /**
         * @brief System drawing a sprite.
         *
         * @param sprite  The sprite parameters (scale, rotation, tint...).
         * @param texture The texture to draw.
         * @param pos     The position where the sprite is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning @p texture must stay valid for the whole call.
         */
        static void systemDrawSprite2D(const Sprite2D &sprite,
            const Texture2D &texture, const Position2D &pos);
    };

} // namespace RType

#endif /* !SPRITE2D_HPP_ */
