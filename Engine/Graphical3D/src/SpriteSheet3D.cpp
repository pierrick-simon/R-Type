/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet3D
*/

#include "SpriteSheet3D.hpp"

namespace RType {
    void SpriteSheet3D::systemAnimateSpriteSheet3D(SpriteSheet3D &sprite,
        Model3D &model, ModelAnimation3D &anim, const Clock &clock)
    {
        sprite.sleep += clock.getElapsed();
        if (sprite.sleep > sprite.frameDuration && sprite.animation) {
            sprite.currentFrame++;
            sprite.sleep = std::chrono::milliseconds(0);
        }
        if (sprite.currentFrame >= anim.frameCount(sprite.index))
            sprite.currentFrame = 0.0f;
        model.UpdateAnimation(
            anim.getAnimation(sprite.index), sprite.currentFrame);
    }

    void SpriteSheet3D::systemMasterAnimateSpriteSheet3D(
        GameManager &gm, const MasterClock &clock)
    {
        for (auto [_, s, m, a] :
            gm.view<SpriteSheet3D, Model3D, ModelAnimation3D>().getVector())
            systemAnimateSpriteSheet3D(s, m, a, clock);
    }

    void SpriteSheet3D::systemDrawSpriteSheet3D(GameManager &gm)
    {
        for (auto [_, s, m, pos] :
            gm.view<SpriteSheet3D, Model3D, Position3D>().getVector()) {
            ::DrawModel(m, {pos.x, pos.y, pos.z}, s.scale, s.color);
        }
    }
}; // namespace RType
