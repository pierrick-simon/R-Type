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

    /**
     * @brief Description of one asset to load.
     *
     * The @c type is matched against the static @c TYPE member of each asset
     * class handled by the AssetManager, which decides what class is
     * constructed from @c path.
     */
    struct AssetEntry {
        int id;                ///< Unique id used to retrieve the asset later.
        std::string path;      ///< Path of the file to load.
        std::string_view type; ///< Asset type tag (e.g. "texture", "sound").
    };

    /**
     * @brief List of assets to load in one call to AssetManager::loadAssets.
     */
    using AssetList = std::vector<AssetEntry>;

    /**
     * @brief Satisfied when @p T is the same type as at least one of @p Ts.
     *
     * @tparam T  The type to look for.
     * @tparam Ts The types to search in.
     */
    template<typename T, typename... Ts>
    concept OneOf = (std::same_as<T, Ts> || ...);

    /**
     * @brief Requirements for a type to be stored in an AssetManager.
     *
     * A type @c C is Unloadable if:
     *  - it can be constructed from a @c const std::string & (the file path),
     *  - it has a @c void @c unload(GameManager &) member function, called
     *    before the asset is destroyed so it can release every entity or
     *    component depending on it,
     *  - it has a static @c TYPE member convertible to @c std::string_view,
     *    used to route an AssetEntry to this type.
     *
     * @tparam C The type to check.
     */
    template<typename C>
    concept Unloadable = std::constructible_from<C, const std::string &> &&
        requires(C &c, GameManager &gm) {
            { c.unload(gm) } -> std::same_as<void>;
        } && requires {
            { C::TYPE } -> std::convertible_to<std::string_view>;
        };

    /**
     * @brief Stores assets of several different types, each under an integer
     *        id, and keeps their literal type.
     *
     * Internally holds one @c std::unordered_map<int, T> per asset type, so
     * get() returns a reference to the real object (Texture2D, Font,
     * Decoder, ...) with no virtual call or cast.
     *
     * An asset is created from an AssetEntry: the entry's @c type is compared
     * with the @c TYPE of each type in @p Ts, and the first match is
     * constructed from the entry's path.
     *
     * @note Only one AssetManager can be created for the whole program (see
     *       the constructor).
     * @warning The manager does not unload anything on destruction, because
     *          unloading needs a GameManager. Call unloadAll() before it is
     *          destroyed.
     *
     * @tparam Ts The asset types handled by this manager. Each one must
     *            satisfy Unloadable and must be unique.
     */
    template<Unloadable... Ts>
    class AssetManager {
    public:
        /**
         * @brief Create the manager and optionally load an initial list of
         *        assets.
         *
         * @param gm   The GameManager holding the entities and components.
         * @param list Assets to load immediately (empty by default).
         *
         * @throw EngineException If a manager has already been created, or
         *        if an entry of @p list cannot be loaded.
         */
        AssetManager(GameManager &gm, const AssetList &list = {})
        {
            static bool onlyOne = true;
            if (!onlyOne)
                throw EngineException("Window can be created only once");
            onlyOne = false;
            loadAssets(gm, list);
        }

        /**
         * @brief Load every asset of a list.
         *
         * Calls loadAsset() for each entry, in order.
         *
         * @param gm   The GameManager holding the entities and components.
         * @param list The assets to load.
         *
         * @throw EngineException If an entry has a type that no asset class
         *        handles. Entries before it stay loaded.
         */
        void loadAssets(GameManager &gm, const AssetList &list)
        {
            for (const auto &[id, path, type] : list)
                loadAsset(gm, id, path, type);
        }

        /**
         * @brief Load one asset under an id.
         *
         * If an asset already exists under @p id, it is unloaded first
         * (see unload()), whatever its type.
         *
         * @param gm   The GameManager holding the entities and components.
         * @param id   Id under which the asset is stored.
         * @param path Path of the file to load.
         * @param type Type tag, compared with the @c TYPE of each type in
         *             @p Ts.
         *
         * @throw EngineException If no asset class handles @p type, or if
         *        the asset constructor fails.
         */
        void loadAsset(GameManager &gm, int id, const std::string &path,
            const std::string_view &type)
        {
            unload(gm, id);
            if (!(tryLoad<Ts>(id, path, type) || ...))
                throw EngineException("No asset type handles '." +
                    std::string(type) + "' (" + path + ")");
        }

        /**
         * @brief Get an asset as its literal type.
         *
         * Only the map of type @p T is searched, so an id belonging to
         * another asset type returns an empty optional.
         *
         * @tparam T The asset type to retrieve. Must be one of @p Ts.
         * @param id Id of the asset.
         *
         * @return A reference to the asset, or @c std::nullopt if there is
         *         no asset of type @p T under @p id.
         *
         * @warning The reference is invalidated when the asset is unloaded.
         *
         * @note Inside a template where the manager type is dependent, call
         *       it as @c manager.template @c get<T>(id).
         */
        template<typename T>
            requires(std::same_as<T, Ts> || ...)
        std::optional<std::reference_wrapper<T>> get(int id)
        {
            auto &map = mapOf<T>();
            auto it = map.find(id);
            return it == map.end() ? std::nullopt
                                   : std::optional {std::ref(it->second)};
        }

        /**
         * @brief Unload the asset stored under an id.
         *
         * Calls the asset's @c unload() then destroys it. Does nothing if
         * the id is unknown.
         *
         * @param gm The GameManager holding the entities and components.
         * @param id Id of the asset to unload.
         */
        void unload(GameManager &gm, int id)
        {
            (unloadFrom<Ts>(gm, id), ...);
        }

        /**
         * @brief Unload every asset of every type.
         *
         * @param gm The GameManager holding the entities and components.
         */
        void unloadAll(GameManager &gm)
        {
            (unloadAllOf<Ts>(gm), ...);
        }

    private:
        /**
         * @brief Map of id to asset for one asset type.
         *
         * @tparam T The asset type.
         */
        template<typename T>
        using Map = std::unordered_map<int, T>;

        /**
         * @brief Get the map storing the assets of type @p T.
         *
         * @tparam T The asset type.
         *
         * @return A reference to the corresponding map.
         */
        template<typename T>
        Map<T> &mapOf()
        {
            return std::get<Map<T>>(_assets);
        }

        /**
         * @brief Load the asset if @p type matches the @c TYPE of @p T.
         *
         * The asset is constructed in place in its map, so @p T does not
         * need to be copyable or movable.
         *
         * @tparam T  The asset type to try.
         * @param id   Id under which the asset is stored.
         * @param path Path of the file to load.
         * @param type Type tag to compare with @c T::TYPE.
         *
         * @return @c true if the asset was loaded, @c false if @p type does
         *         not match @p T.
         */
        template<typename T>
        bool tryLoad(int id, const std::string &path, std::string_view type)
        {
            if (T::TYPE != type)
                return false;
            mapOf<T>().emplace(id, path);
            return true;
        }

        /**
         * @brief Unload and erase the asset @p id from the map of type @p T.
         *
         * Does nothing if the id is not in that map.
         *
         * @tparam T The asset type.
         * @param gm The GameManager holding the entities and components.
         * @param id Id of the asset to unload.
         */
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

        /**
         * @brief Unload and erase every asset of the map of type @p T.
         *
         * @tparam T The asset type.
         * @param gm The GameManager holding the entities and components.
         */
        template<typename T>
        void unloadAllOf(GameManager &gm)
        {
            auto &map = mapOf<T>();
            for (auto &[id, asset] : map)
                asset.unload(gm);
            map.clear();
        }

        std::tuple<Map<Ts>...> _assets; ///< One id -> asset map per type.
    };
} // namespace RType

#endif /* !ASSETMANAGER_HPP_ */
