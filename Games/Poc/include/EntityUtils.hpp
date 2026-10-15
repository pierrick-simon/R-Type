/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** EntityUtils
*/

#ifndef POC_ENTITYUTILS_HPP_
#define POC_ENTITYUTILS_HPP_

#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <vector>

#include "Entity.hpp"

namespace Poc {

    template<typename E>
    std::size_t indexOf(const E &entity)
    {
        if constexpr (std::is_integral_v<E>)
            return static_cast<std::size_t>(entity);
        else
            return static_cast<std::size_t>(entity.index);
    }

    class EntitySet {
    public:
        bool contains(const RType::Entity &entity) const
        {
            return std::find(_entities.begin(), _entities.end(), entity) !=
                _entities.end();
        }

        void insert(const RType::Entity &entity)
        {
            if (!contains(entity))
                _entities.push_back(entity);
        }

        void clear()
        {
            _entities.clear();
        }

        [[nodiscard]] std::size_t size() const
        {
            return _entities.size();
        }

        [[nodiscard]] auto begin() const
        {
            return _entities.begin();
        }

        [[nodiscard]] auto end() const
        {
            return _entities.end();
        }

    private:
        std::vector<RType::Entity> _entities;
    };

} // namespace Poc

#endif /* !POC_ENTITYUTILS_HPP_ */
