/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#ifndef SPRITESHEET2D_HPP_
#define SPRITESHEET2D_HPP_

#include "Clock.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Size2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing an animated sprite sheet.
     *
     * The texture is split into a grid of tiles (#recNb); the system displays
     * one tile at a time and advances to the next one every #frameDuration.
     */
    struct SpriteSheet2D {
        /**
         * @brief Number of tiles in the texture.
         *
         * @warning Members of Size2D are ordered `{height, width}`: `{1, 1}`
         *          means a single tile. Make sure to give the number of rows
         *          first, then the number of columns.
         */
        Size2D recNb = {1, 1};
        std::size_t currentTile = 0;
        std::chrono::milliseconds frameDuration =
            std::chrono::milliseconds(100);
        std::chrono::milliseconds sleep = std::chrono::milliseconds(0);
        Origin2D origin;
        bool animation = true;
        float scale = 1;
        float rotation = 0;
        bool flip = false;
        raylib::Color color = raylib::Color::White();

        /**
         * @brief System animating and drawing a sprite sheet.
         *
         * @param sprite  The sprite sheet parameters and animation state.
         * @param texture The texture containing all the tiles.
         * @param pos     The position where the current tile is drawn.
         * @param clock   The clock providing the elapsed time.
         *
         * @warning Clock::systemClock must have been called beforehand in the
         *          same frame, otherwise the animation will not progress.
         * @warning #recNb must not contain 0 (it would lead to a division by
         *          zero when computing the tile size).
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         */
        static void systemSpriteSheet2D(SpriteSheet2D &sprite,
            const Texture2D &texture, const Position2D &pos,
            const Clock &clock);
    };

} // namespace RType

#endif /* !SPRITESHEET2D_HPP_ */
