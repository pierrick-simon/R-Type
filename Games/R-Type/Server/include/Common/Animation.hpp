/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Animation
*/

#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include "Components.hpp"

namespace RType {
    struct SpriteSheet2D;
}

namespace RType::Server {

    void playOnceAnimation(const PlayOnce &playOnce, SpriteSheet2D &sheet);
} // namespace RType::Server

#endif
