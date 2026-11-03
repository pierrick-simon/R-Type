/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Terrain
*/

#include "Terrain.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <raylib-cpp.hpp>

#include "Components.hpp"
#include "Config.hpp"
#include "Exceptions.hpp"

namespace RType::Server {

    namespace {
        constexpr int PLATFORM_TOLERANCE = 8;

        const std::string &assetPath(GameManager &gm, AssetId id)
        {
            const auto &assets = gm.getAssetsMap();
            const auto it = assets.find(id);

            if (it == assets.end())
                throw EngineException("No such asset: " + std::to_string(id));
            return it->second;
        }

        CollisionMask makeMask(const raylib::Image &image, int left, int width,
            const std::function<bool(raylib::Color)> &solid)
        {
            CollisionMask mask {width, image.height, {}};

            mask.solid.resize(std::size_t(width) * std::size_t(image.height));
            for (int y = 0; y < image.height; ++y)
                for (int x = 0; x < width; ++x)
                    mask.solid[std::size_t(y) * std::size_t(width) +
                        std::size_t(x)] = solid(image.GetColor(left + x, y));
            return mask;
        }

        std::vector<int> floorTops(const CollisionMask &mask)
        {
            std::vector<int> tops(std::size_t(mask.width), mask.height);

            for (int x = 0; x < mask.width; ++x) {
                for (int y = mask.height / 2; y < mask.height; ++y) {
                    if (mask.isSolid(x, y)) {
                        tops[std::size_t(x)] = y;
                        break;
                    }
                }
                if (x > 0 && tops[std::size_t(x)] == mask.height)
                    tops[std::size_t(x)] = tops[std::size_t(x) - 1];
            }
            return tops;
        }

        std::vector<Platform> extractPlatforms(const CollisionMask &mask)
        {
            const std::vector<int> tops = floorTops(mask);
            std::vector<Platform> platforms;
            std::size_t start = 0;
            int low = tops[0];
            int high = tops[0];

            for (std::size_t x = 1; x < tops.size(); ++x) {
                const int top = tops[x];

                if (std::max(high, top) - std::min(low, top) <=
                    PLATFORM_TOLERANCE) {
                    low = std::min(low, top);
                    high = std::max(high, top);
                    continue;
                }
                platforms.push_back({float(start), float(x), float(low)});
                start = x;
                low = top;
                high = top;
            }
            platforms.push_back({float(start), float(tops.size()), float(low)});
            return platforms;
        }

    } // namespace

    Entity createTerrain(GameManager &gm)
    {
        try {
            const raylib::Image image(assetPath(gm, Asset::LEVEL1_MASK));
            CollisionMask mask =
                makeMask(image, 0, image.width, [](raylib::Color c) {
                    return c.r != 0 || c.g != 0 || c.b != 0;
                });
            const Entity terrain = gm.createEntity();
            std::vector<Platform> platforms = extractPlatforms(mask);

            gm.addComponent<Terrain>(
                terrain, Terrain {std::move(mask), std::move(platforms)});
            return terrain;
        } catch (raylib::RaylibException &e) {
            throw EngineException(e.what());
        }
    }

    Entity createSpriteMasks(GameManager &gm, AssetId asset, int tiles)
    {
        try {
            const raylib::Image image(assetPath(gm, asset));
            const float tileWidth = float(image.width) / float(tiles);
            SpriteMasks masks {asset, {}};
            const Entity entity = gm.createEntity();

            for (int i = 0; i < tiles; ++i) {
                const int left = int(float(i) * tileWidth);
                const int right = int(float(i + 1) * tileWidth);

                masks.tiles.push_back(
                    makeMask(image, left, right - left, [](raylib::Color c) {
                        return c.a != 0;
                    }));
            }
            gm.addComponent<SpriteMasks>(entity, std::move(masks));
            return entity;
        } catch (raylib::RaylibException &e) {
            throw EngineException(e.what());
        }
    }

    const CollisionMask *spriteMask(
        GameManager &gm, AssetId asset, std::size_t tile)
    {
        for (auto [_, masks] : gm.view<SpriteMasks>().getVector())
            if (masks.asset == asset && tile < masks.tiles.size())
                return &masks.tiles[tile];
        return nullptr;
    }

    std::optional<Platform> platformAt(GameManager &gm, float screenX)
    {
        for (auto [_, terrain] : gm.view<Terrain>().getVector()) {
            for (auto [entity, background, pos] :
                gm.view<Background, Position2D>().getVector()) {
                (void) entity;
                (void) background;
                const float levelX = (screenX - pos.x) / SCALE;
                const auto platform = std::ranges::find_if(
                    terrain.platforms, [levelX](const Platform &p) {
                        return p.left <= levelX && levelX < p.right;
                    });

                if (platform == terrain.platforms.end())
                    continue;
                return Platform {pos.x + platform->left * SCALE,
                    pos.x + platform->right * SCALE,
                    platform->top * SCALE};
            }
        }
        return std::nullopt;
    }

    bool touchesTerrain(
        GameManager &gm, const Position2D &pos, const CollisionMask &shape)
    {
        for (auto [_, terrain] : gm.view<Terrain>().getVector()) {
            for (auto [entity, background, bgPos] :
                gm.view<Background, Position2D>().getVector()) {
                (void) entity;
                (void) background;
                const float originX = (pos.x - bgPos.x) / SCALE;
                const float originY = (pos.y - bgPos.y) / SCALE;

                for (int y = 0; y < shape.height; ++y) {
                    for (int x = 0; x < shape.width; ++x) {
                        if (!shape.isSolid(x, y))
                            continue;
                        const auto levelX =
                            int(std::floor(originX + float(x) + 0.5f));
                        const auto levelY =
                            int(std::floor(originY + float(y) + 0.5f));

                        if (terrain.mask.isSolid(levelX, levelY))
                            return true;
                    }
                }
            }
        }
        return false;
    }

} // namespace RType::Server
