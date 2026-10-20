/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Texture
*/

#include "Texture.hpp"
#include <vector>

#include "Exceptions.hpp"

namespace RType::Server {

    Texture2D loadPixelTexture(GameManager &gm, AssetId id)
    {
        const auto &assets = gm.getAssetsMap();
        const auto it = assets.find(id);

        if (it == assets.end())
            throw EngineException("No such asset: " + std::to_string(id));
        Texture2D texture(it->second);

        texture.SetFilter(TEXTURE_FILTER_POINT);
        return texture;
    }

    void unloadTextures(GameManager &gm)
    {
        std::vector<Entity> textured;

        gm.view<Texture2D>().each(
            [&textured](Entity e, Texture2D &) { textured.push_back(e); });
        for (const Entity e : textured)
            gm.destroyEntity(e);
    }

} // namespace RType::Server
