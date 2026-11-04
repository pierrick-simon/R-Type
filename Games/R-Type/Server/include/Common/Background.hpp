/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Background
*/

#ifndef BACKGROUND_HPP_
#define BACKGROUND_HPP_

#include "Components.hpp"
#include "GameManager.hpp"
#include "Position2D.hpp"

namespace RType::Server {

    void createBackground(GameManager &gm);
    void resetBackground(GameManager &gm);
    void backgroundScroll(GameManager &gm);
    [[nodiscard]] bool levelEnded(GameManager &gm);
} // namespace RType::Server

#endif
