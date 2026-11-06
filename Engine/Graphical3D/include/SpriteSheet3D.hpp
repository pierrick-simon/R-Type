/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteShee3D
*/

#ifndef SPRITESHEET3D_HPP_
#define SPRITESHEET3D_HPP_

#include <optional>
#include "Clock.hpp"
#include "GameManager.hpp"
#include "ModelAnimation3D.hpp"
#include "Position3D.hpp"
#include "Sprite3D.hpp"

namespace RType {
    /**
     * @brief Animated 3D sprite component driven by model animations.
     */
    struct SpriteSheet3D {
        int index = 0;
        int currentFrame = 0;
        std::chrono::milliseconds frameDuration =
            std::chrono::milliseconds(100);
        std::chrono::milliseconds sleep = std::chrono::milliseconds(0);
        bool animation = true;
        float scale = 1;
        raylib::Color color = raylib::Color::White();

        /**
         * @brief System advancing the animation of a single sprite sheet.
         * @param sprite Sprite sheet to animate.
         * @param model Model the animation is applied to.
         * @param anim Animations available for the model.
         * @param clock Clock used to measure elapsed time.
         */
        static void systemAnimateSpriteSheet3D(SpriteSheet3D &sprite,
            Model3D &model, ModelAnimation3D &anim, const Clock &clock);

        /**
         * @brief System animating all the entities having a SpriteSheet3D.
         * @param gm Game manager giving access to the entities and
         * components.
         * @param clock Master clock used to measure elapsed time.
         */
        static void systemMasterAnimateSpriteSheet3D(
            GameManager &gm, const MasterClock &clock);

        /**
         * @brief System drawing all the entities having a SpriteSheet3D.
         * @param gm Game manager giving access to the entities and
         * components.
         */
        static void systemDrawSpriteSheet3D(GameManager &gm);
    };

} // namespace RType

#endif /* !SPRITESHEET3D_HPP_ */
