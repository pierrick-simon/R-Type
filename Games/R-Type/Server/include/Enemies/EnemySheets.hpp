/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EnemySheets.hpp
*/

#ifndef ENEMYSHEET_HPP_
#define ENEMYSHEET_HPP_

namespace RType {

    /**
     * @brief Description of a REGULAR strip (all cells have the same size).
     *
     * The strips are produced from the original irregular sheets with
     * tools/repack_sheet.py, which prints frames / cellW / cellH.
     * hitboxW / hitboxH are in sprite pixels (before SPRITE_SCALE) and the
     * hitbox is centered in the cell.
     */
    struct SheetInfo {
        const char *file;
        int frames;
        int cellW;
        int cellH;
        int frameMs;
        int hitboxW;
        int hitboxH;
    };

    // Same relative root as the player texture in Main.cpp.
    inline constexpr SheetInfo SCOUT_SHEET = {
        "R-Type/sprites/enemies/scout.png", 16, 21, 24, 70, 21, 14};

    inline constexpr SheetInfo EXPLOSION_SHEET = {
        "R-Type/sprites/enemies/explosion.png", 7, 32, 31, 60, 0, 0};
} // namespace RType

#endif /* !ENEMYSHEET_HPP_ */
