/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Background
*/

#include "Background.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "Sprite2D.hpp"
#include "Texture.hpp"
#include "Velocity2D.hpp"

namespace RType::Server {

    void backgroundLoop(const Background &, Position2D &pos)
    {
        if (pos.x <= -BACKGROUND_PERIOD)
            pos.x += 2.f * BACKGROUND_PERIOD;
    }

    void createBackground(GameManager &gm)
    {
        for (int i = 0; i < 2; i++) {
            auto background = gm.createEntity();

            gm.addComponent<Background>(background);
            gm.addComponent<Texture2D>(
                background, loadPixelTexture(BACKGROUND_PATH));
            gm.addComponent<Sprite2D>(background, Origin2D(), SCALE);
            gm.addComponent<Position2D>(
                background, static_cast<float>(i) * BACKGROUND_PERIOD, 0.f);
            gm.addComponent<Velocity2D>(background, -BACKGROUND_SPEED, 0.f);
            gm.addComponent<Clock>(background);
        }
    }

} // namespace RType::Server
