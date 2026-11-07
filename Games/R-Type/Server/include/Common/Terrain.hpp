/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Terrain
*/

#ifndef TERRAIN_HPP_
#define TERRAIN_HPP_

#include <optional>
#include <vector>
#include "Assets.hpp"
#include "Config.hpp"
#include "GameManager.hpp"
#include "Position2D.hpp"

namespace RType::Server {

    struct CollisionMask {
        int width = 0;
        int height = 0;
        std::vector<bool> solid;

        [[nodiscard]] bool isSolid(int x, int y) const
        {
            return x >= 0 && y >= 0 && x < width && y < height &&
                solid[std::size_t(y) * std::size_t(width) + std::size_t(x)];
        }
    };

    struct Platform {
        float left = 0.f;
        float right = 0.f;
        float top = 0.f;
    };

    struct Terrain {
        CollisionMask mask;
        std::vector<Platform> platforms;
    };

    struct SpriteMasks {
        AssetId asset = 0;
        std::vector<CollisionMask> tiles;
    };

    Entity createTerrain(GameManager &gm);

    Entity createSpriteMasks(GameManager &gm, AssetId asset, int tiles);

    const CollisionMask *spriteMask(
        GameManager &gm, AssetId asset, std::size_t tile);

    std::optional<Platform> platformAt(GameManager &gm, float screenX);

    /** @brief Whether a sprite drawn at the given scale overlaps a solid pixel
     * of the level. */
    bool touchesTerrain(GameManager &gm, const Position2D &pos,
        const CollisionMask &shape, float shapeScale = SCALE);

    /** @brief Whether a screen rectangle overlaps a solid pixel of the level.
     */
    bool rectTouchesTerrain(
        GameManager &gm, float x, float y, float width, float height);
} // namespace RType::Server

#endif
