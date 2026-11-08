/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Draw3D
*/

#ifndef DRAW3D_HPP_
#define DRAW3D_HPP_

#include "AssetManager.hpp"
#include "Camera3D.hpp"
#include "GameManager.hpp"
#include "Model3D.hpp"
#include "Sprite3D.hpp"
#include "SpriteSheet3D.hpp"
#include "Window.hpp"

namespace RType {
    /**
     * @brief Groups the systems responsible for the 3D rendering pass.
     */
    struct Draw3D {
        /**
         * @brief System running the 3D drawing pass.
         * @param gm Game manager giving access to the entities and
         * components.
         */
        static void systemDraw3D(GameManager &gm);

        template<Unloadable... Ts>
            requires OneOf<Model3D, Ts...>
        static void systemDrawManager3D(
            GameManager &gm, AssetManager<Ts...> &assetManager)
        {
            for (auto [_, camera] : gm.view<Camera3D>().getVector()) {
                camera.BeginMode();
                Sprite3D::systemDrawManagerSprite3D<Ts...>(gm, assetManager);
                SpriteSheet3D::systemDrawManagerSpriteSheet3D<Ts...>(
                    gm, assetManager);
                camera.EndMode();
            }
        }
    };
} // namespace RType

#endif /* !DRAW3D_HPP_ */
