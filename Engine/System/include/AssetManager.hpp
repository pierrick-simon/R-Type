/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AssetManager
*/

#ifndef ASSETMANAGER_HPP_
#define ASSETMANAGER_HPP_

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "Exceptions.hpp"
#include "GameManager.hpp"

namespace RType {

    struct AssetEntry {
        int id;
        std::string path;
        std::string_view type;
    };

    using AssetList = std::vector<AssetEntry>;

    template<typename T, typename... Ts>
    concept OneOf = (std::same_as<T, Ts> || ...);

    template<typename C>
    concept Unloadable = std::constructible_from<C, const std::string &> &&
        requires(C &c, GameManager &gm) {
            { c.unload(gm) } -> std::same_as<void>;
        } && requires {
            { C::TYPE } -> std::convertible_to<std::string_view>;
        };

    template<Unloadable... Ts>
    class AssetManager {
    public:
        AssetManager(GameManager &gm, const AssetList &list = {})
        {
            static bool onlyOne = true;
            if (!onlyOne)
                throw EngineException("Window can be created only once");
            onlyOne = false;
            loadAssets(gm, list);
        }

        void loadAssets(GameManager &gm, const AssetList &list)
        {
            for (const auto &[id, path, type] : list)
                loadAsset(gm, id, path, type);
        }

        void loadAsset(GameManager &gm, int id, const std::string &path,
            const std::string_view &type)
        {
            unload(gm, id);
            if (!(tryLoad<Ts>(id, path, type) || ...))
                throw EngineException("No asset type handles '." +
                    std::string(type) + "' (" + path + ")");
        }

        template<typename T>
            requires(std::same_as<T, Ts> || ...)
        std::optional<std::reference_wrapper<T>> get(int id)
        {
            auto &map = mapOf<T>();
            auto it = map.find(id);
            return it == map.end() ? std::nullopt
                                   : std::optional {std::ref(it->second)};
        }

        void unload(GameManager &gm, int id)
        {
            (unloadFrom<Ts>(gm, id), ...);
        }

        void unloadAll(GameManager &gm)
        {
            (unloadAllOf<Ts>(gm), ...);
        }

    private:
        template<typename T>
        using Map = std::unordered_map<int, T>;

        template<typename T>
        Map<T> &mapOf()
        {
            return std::get<Map<T>>(_assets);
        }

        template<typename T>
        bool tryLoad(int id, const std::string &path, std::string_view type)
        {
            if (T::TYPE != type)
                return false;
            mapOf<T>().emplace(id, path);
            return true;
        }

        template<typename T>
        void unloadFrom(GameManager &gm, int id)
        {
            auto &map = mapOf<T>();
            auto it = map.find(id);
            if (it == map.end())
                return;
            it->second.unload(gm);
            map.erase(it);
        }

        template<typename T>
        void unloadAllOf(GameManager &gm)
        {
            auto &map = mapOf<T>();
            for (auto &[id, asset] : map)
                asset.unload(gm);
            map.clear();
        }

        std::tuple<Map<Ts>...> _assets;
    };
} // namespace RType

#endif /* !ASSETMANAGER_HPP_ */
