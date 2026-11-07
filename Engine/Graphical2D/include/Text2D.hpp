/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2d
*/

#ifndef TEXT2D_HPP_
#define TEXT2D_HPP_

#include <optional>
#include <raylib-cpp.hpp>
#include <string>
#include "AssetManager.hpp"
#include "Font.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"

namespace RType {

    /**
     * @brief Text component drawn on screen at a given position.
     */
    struct Text2D {
        Text2D() = default;
        Text2D(int assetId) :
            assetId(assetId) {};

        float fontSize = 10;
        std::string str = "Hello World";
        raylib::Color color = raylib::Color::White();
        float rotation = 0;
        float spacing = 0;
        Origin2D origin;
        std::optional<int> assetId = std::nullopt;

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
            const Text2D &text, const Font &font, const Position2D &pos);

        template<Unloadable... Ts>
            requires OneOf<Font, Ts...>
        static void systemDrawManagerText2D(
            GameManager &gm, AssetManager<Ts...> &assetManager)
        {
            for (auto [_, s, p] : gm.view<Text2D, Position2D>().getVector()) {
                if (!s.assetId)
                    continue;
                auto font = assetManager.template get<Font>(s.assetId.value());
                if (!font)
                    continue;
                systemDrawText2D(s, font->get(), p);
            }
        }
    };

} // namespace RType

#endif /* !TEXT2D_HPP_ */
