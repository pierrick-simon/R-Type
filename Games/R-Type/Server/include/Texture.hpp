/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture
*/

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include "Assets.hpp"
#include "GameManager.hpp"
#include "Texture2D.hpp"

namespace RType::Server {

    Texture2D loadPixelTexture(GameManager &gm, AssetId id);
    void unloadTextures(GameManager &gm);
} // namespace RType::Server

#endif
