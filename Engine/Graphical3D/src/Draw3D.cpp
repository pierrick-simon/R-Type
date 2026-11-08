/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Draw3D
*/

#include "Draw3D.hpp"
#include "Sprite3D.hpp"
#include "SpriteSheet3D.hpp"

namespace RType {
    void Draw3D::systemDraw3D(GameManager &gm)
    {
        for (auto [_, camera] : gm.view<Camera3D>().getVector()) {
            camera.BeginMode();
            Sprite3D::systemDrawSprite3D(gm);
            SpriteSheet3D::systemDrawSpriteSheet3D(gm);
            camera.EndMode();
        }
    }
} // namespace RType
