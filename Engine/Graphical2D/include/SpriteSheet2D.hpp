/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#ifndef SPRITESHEET2D_HPP_
#define SPRITESHEET2D_HPP_

#include <optional>
#include "AssetManager.hpp"
#include "Clock.hpp"
#include "GameManager.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Size2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing an animated sprite sheet.
     *
     * The texture is split into a grid of tiles (recNb()); the system displays
     * one tile at a time and advances to the next one every frameDuration().
     *
     * @note The systems write directly to the members, so the local animation
     *       (currentTile advancing each frame) is NOT flagged as an update.
     *       Only changes made through the accessors are.
     */
    struct SpriteSheet2D {
        /** @brief SpriteSheet2D default constructor
         */
        SpriteSheet2D() = default;
        SpriteSheet2D(Size2D recNb, unsigned int frameDuration, float scale) :
            recNb(recNb), frameDuration(frameDuration), scale(scale) {};
        SpriteSheet2D(Size2D recNb, std::size_t currentTile,
            unsigned int frameDuration, float scale, bool animation = true) :
            recNb(recNb),
            currentTile(currentTile),
            frameDuration(frameDuration),
            scale(scale),
            animation(animation) {};
        SpriteSheet2D(int assetId) :
            assetId(assetId) {};

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
        std::optional<int> assetId = std::nullopt;

        /**
         * @brief System drawing a sprite sheet.
         *
         * @param sprite  The sprite sheet parameters and animation state.
         * @param texture The texture containing all the tiles.
         * @param pos     The position where the current tile is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning recNb() must not contain 0 (it would lead to a division by
         *          zero when computing the tile size).
         */
        static void systemDrawSpriteSheet2D(SpriteSheet2D &sprite,
            const Texture2D &texture, const Position2D &pos);

        template<Unloadable... Ts>
            requires OneOf<Texture2D, Ts...>
        static void systemDrawManagerSpriteSheet2D(
            GameManager &gm, AssetManager<Ts...> &assetManager)
        {
            for (auto [_, s, p] :
                gm.view<SpriteSheet2D, Position2D>().getVector()) {
                if (!s.assetId)
                    continue;
                auto texture =
                    assetManager.template get<Texture2D>(s.assetId.value());
                if (!texture)
                    continue;
                systemDrawSpriteSheet2D(s, texture->get(), p);
            }
        }

        /**
         * @brief System animating a sprite sheet.
         *
         * @param sprite  The sprite sheet parameters and animation state.
         * @param clock   The clock providing the elapsed time.
         *
         * @warning Clock::systemClock must have been called beforehand in the
         *          same frame, otherwise the animation will not progress.
         */
        static void systemAnimateSpriteSheet2D(
            SpriteSheet2D &sprite, const Clock &clock);
        static void systemMasterAnimateSpriteSheet2D(
            GameManager &gm, const MasterClock &clock);
    };

} // namespace RType

#endif /* !SPRITESHEET2D_HPP_ */
