/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#ifndef SPRITE2D_HPP_
#define SPRITE2D_HPP_

#include "AssetManager.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing how a whole texture is drawn.
     *
     * @note On a non-const object, even a read through the overloaded accessor
     *       flags the property. Use a const reference to read without flagging.
     */
    struct Sprite2D {

        /**
         * @brief Default constructor
         */
        Sprite2D() = default;
        Sprite2D(float scale) :
            scale(scale) {};
        Sprite2D(int assetId) :
            assetId(assetId) {};

        Origin2D origin;
        float scale = 1.f;
        float rotation = 0.f;
        bool flip = false;
        raylib::Color color = raylib::Color::White();
        std::optional<int> assetId = std::nullopt;

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

        template<Unloadable... Ts>
            requires OneOf<Texture2D, Ts...>
        static void systemDrawManagerSprite2D(
            GameManager &gm, AssetManager<Ts...> &assetManager)
        {
            for (auto [_, s, p] : gm.view<Sprite2D, Position2D>().getVector()) {
                if (!s.assetId)
                    continue;
                auto texture =
                    assetManager.template get<Texture2D>(s.assetId.value());
                if (!texture)
                    continue;
                systemDrawSprite2D(s, texture->get(), p);
            }
        }
    };

} // namespace RType

#endif /* !SPRITE2D_HPP_ */
