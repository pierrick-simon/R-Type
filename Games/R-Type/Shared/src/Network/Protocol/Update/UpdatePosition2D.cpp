/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdatePosition2D
*/

#include "UpdatePosition2D.hpp"
#include "SyncField.hpp"

namespace RType {
    void UpdatePosition2D::systemUpdatePosition2D(
        UpdatePosition2D &update, const Position2D &pos)
    {
        const bool xChanged = syncField(update.x, pos.x);
        const bool yChanged = syncField(update.y, pos.y);

        update._update = update._update || xChanged || yChanged;
    }
}; // namespace RType
