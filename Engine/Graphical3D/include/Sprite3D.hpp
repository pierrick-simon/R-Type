/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite3D
*/

#ifndef SPRITE3D_HPP_
#define SPRITE3D_HPP_

#include "AssetManager.hpp"
#include "Camera3D.hpp"
#include "GameManager.hpp"
#include "Model3D.hpp"
#include "Position3D.hpp"

namespace RType {
    /**
     * @brief Static 3D sprite component (a model drawn with a scale and a
     * tint color).
     */
    struct Sprite3D {
        Sprite3D() = default;
        Sprite3D(int assetId) :
            assetId(assetId) {};

        float scale = 1;
        raylib::Color color = raylib::Color::White();
        std::optional<int> assetId = std::nullopt;

        /**
         * @brief System drawing all the entities having a Sprite3D.
         * @param gm Game manager giving access to the entities and
         * components.
         */
        static void systemDrawSprite3D(GameManager &gm);

        template<Unloadable... Ts>
            requires OneOf<Model3D, Ts...>
        static void systemDrawManagerSprite3D(
            GameManager &gm, AssetManager<Ts...> &assetManager)
        {
            for (auto [_, s, pos] :
                gm.view<Sprite3D, Position3D>().getVector()) {
                if (!s.assetId)
                    continue;
                auto model3D =
                    assetManager.template get<Model3D>(s.assetId.value());
                if (!model3D)
                    continue;
                ::DrawModel(model3D, {pos.x, pos.y, pos.z}, s.scale, s.color);
            }
        }
    };

} // namespace RType

#endif /* !SPRITE3D_HPP_ */
