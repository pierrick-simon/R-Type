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
#include "GameManager.hpp"
#include "Position2D.hpp"

namespace RType::Server {

    /** @brief Grid of solid pixels of an image. */
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

    /**
     * @brief Flat piece of floor, from left (inclusive) to right
     * (exclusive), whose surface is at height top.
     */
    struct Platform {
        float left = 0.f;
        float right = 0.f;
        float top = 0.f;
    };

    /** @brief Decor of the level, in level mask pixels. */
    struct Terrain {
        CollisionMask mask;
        std::vector<Platform> platforms;
    };

    /** @brief Opaque pixels of each tile of a one-row sprite sheet. */
    struct SpriteMasks {
        AssetId asset = 0;
        std::vector<CollisionMask> tiles;
    };

    /**
     * @brief Creates the entity holding the decor, extracted from the level
     * collision mask.
     */
    Entity createTerrain(GameManager &gm);

    /**
     * @brief Creates the entity holding the opaque pixels of each tile of a
     * one-row sprite sheet, used for pixel-perfect collisions.
     */
    Entity createSpriteMasks(GameManager &gm, AssetId asset, int tiles);

    /**
     * @return The opaque pixels of a tile of a sprite sheet loaded with
     * createSpriteMasks, or nullptr if there is none.
     */
    const CollisionMask *spriteMask(
        GameManager &gm, AssetId asset, std::size_t tile);

    /**
     * @brief Platform under a screen x coordinate, in screen coordinates,
     * following the scrolling of the background.
     * @return The platform, or std::nullopt if there is no terrain there.
     */
    std::optional<Platform> platformAt(GameManager &gm, float screenX);

    /**
     * @brief Checks whether a shape drawn at pos overlaps the decor.
     * @warning The shape must be drawn with the same scale as the
     * background, so that one of its pixels covers one pixel of the mask.
     */
    bool touchesTerrain(
        GameManager &gm, const Position2D &pos, const CollisionMask &shape);
} // namespace RType::Server

#endif
