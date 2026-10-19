/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySprites
*/

#include "EnemySprites.hpp"
#include "EnemyDefs.hpp"
#include "GameComponents.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType::Game {

    void attachEnemySprite(GameManager &gm, Entity enemy)
    {
        const Assets::SheetInfo &sheet =
            *enemyDef(gm.getComponent<Enemy>(enemy).type).sheet;

        SpriteSheet2D sprite;
        sprite.recNb = {1, sheet.frames}; // Size2D is {height, width}
        sprite.frameDuration = std::chrono::milliseconds(sheet.frameMs);
        sprite.scale = SPRITE_SCALE;
        sprite.origin = {float(sheet.cellW) * SPRITE_SCALE / 2.f,
            float(sheet.cellH) * SPRITE_SCALE / 2.f};

        gm.addComponent<Texture2D>(enemy, sheet.file);
        gm.addComponent<SpriteSheet2D>(enemy, sprite);
    }

} // namespace RType::Game
