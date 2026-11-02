/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Background
*/

#include "Background.hpp"

#include <algorithm>

#include "Assets.hpp"
#include "Config.hpp"
#include "Sprite2D.hpp"
#include "Texture.hpp"
#include "UpdateClientEntity.hpp"
#include "UpdatePosition2D.hpp"
#include "UpdateSprite2D.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    float levelScroll(GameManager &gm)
    {
        float scroll = 0.f;

        for (auto [_, background, pos] :
            gm.view<Background, Position2D>().getVector())
            scroll = std::min(scroll, pos.x);
        return scroll;
    }

    void backgroundScroll(GameManager &gm)
    {
        const float overshoot = LEVEL_END_X - levelScroll(gm);

        if (overshoot < 0.f)
            return;
        // Stops on the boss room, the level's last screen.
        for (auto [_, background, pos, velocity] :
            gm.view<Background, Position2D, Velocity2D>().getVector()) {
            pos.x += overshoot;
            velocity.x = 0.f;
        }
    }

    bool levelEnded(GameManager &gm)
    {
        return levelScroll(gm) <= LEVEL_END_X;
    }

    void resetBackground(GameManager &gm)
    {
        float index = 0.f;

        gm.view<Background, Position2D, Velocity2D>().each(
            [&index](
                Entity, Background &, Position2D &pos, Velocity2D &velocity) {
                pos.x = index * BACKGROUND_PERIOD;
                velocity.x = -BACKGROUND_SPEED;
                index += 1.f;
            });
    }

    void createBackground(GameManager &gm)
    {
        for (int i = 0; i < 2; i++) {
            auto background = gm.createEntity();

            gm.addComponent<Background>(background);
            gm.addComponent<Sprite2D>(background, SCALE);
            gm.addComponent<Position2D>(
                background, static_cast<float>(i) * BACKGROUND_PERIOD, 0.f);
            gm.addComponent<Velocity2D>(background, -BACKGROUND_SPEED, 0.f);
            gm.addComponent<UpdateClientEntity>(background,
                EntityType::SPRITE,
                Asset::LEVEL1_BACKGROUND,
                gm,
                background);
        }
    }

} // namespace RType::Server
