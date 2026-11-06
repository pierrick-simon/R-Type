/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite3D
*/

#include "Sprite3D.hpp"
#include <iostream>

namespace RType {
    void Sprite3D::systemDrawSprite3D(GameManager &gm)
    {
        for (auto [_, s, m, pos] :
            gm.view<Sprite3D, Model3D, Position3D>().getVector()) {
            ::DrawModel(m, {pos.x, pos.y, pos.z}, s.scale, s.color);
        }
    }
}; // namespace RType
