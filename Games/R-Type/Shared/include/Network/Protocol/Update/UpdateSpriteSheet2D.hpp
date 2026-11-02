/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateSpriteSheet2D
*/

#ifndef UPDATESPRITESHEET2D_HPP_
#define UPDATESPRITESHEET2D_HPP_

#include "Property.hpp"
#include "SpriteSheet2D.hpp"

namespace RType {
    class UpdateSpriteSheet2D : public SpriteSheet2D {
    public:
        /**
         * @brief Current value of a property, ready to be sent in a SET packet.
         *
         * @param prop The property to read.
         * @return The value, or std::nullopt if this component has no such
         * property.
         */
        [[nodiscard]] std::optional<SetValue> getProperty(Property prop) const;

        /** @return The changed since the last clean
         */
        [[nodiscard]] const std::vector<Property> &getUpdate() const
        {
            return _update;
        }

        /** @return The default update
         */
        [[nodiscard]] static const std::vector<Property> &getDefaultUpdate()
        {
            return _default;
        }

        /** @brief Clear update
         */
        void clearUpdate()
        {
            _update.clear();
        }

        /**
         * @brief System to update the components UpdateSpriteSheet2D
         */
        static void systemUpdateSpriteSheet2D(
            UpdateSpriteSheet2D &update, const SpriteSheet2D &ss);

    private:
        std::vector<Property> _update;
        static const std::vector<Property> _default;
    };
} // namespace RType

#endif /* !UPDATESPRITESHEET2D_HPP_ */
