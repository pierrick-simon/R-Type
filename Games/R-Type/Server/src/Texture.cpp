/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture
*/

#include "Texture.hpp"

namespace RType::Server {

    Texture2D loadPixelTexture(const char *path)
    {
        Texture2D texture(path);

        texture.SetFilter(TEXTURE_FILTER_POINT);
        return texture;
    }

} // namespace RType::Server
