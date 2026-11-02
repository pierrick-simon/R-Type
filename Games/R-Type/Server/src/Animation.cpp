/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Animation
*/

#include "Animation.hpp"
#include "SpriteSheet2D.hpp"

namespace RType::Server {

    void playOnceAnimation(const PlayOnce &, SpriteSheet2D &sheet)
    {
        const auto last =
            std::size_t(sheet.recNb.width * sheet.recNb.height) - 1;

        if (sheet.currentTile >= last) {
            sheet.currentTile = last;
            sheet.animation = false;
        }
    }

} // namespace RType::Server
