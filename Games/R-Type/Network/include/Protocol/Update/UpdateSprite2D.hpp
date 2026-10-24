/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateSprite2D
*/

#ifndef UPDATESPRITE2D_HPP_
#define UPDATESPRITE2D_HPP_

#include "Property.hpp"
#include "Sprite2D.hpp"

namespace RType {
    class UpdateSprite2D : public Sprite2D {
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
         * @brief System to update the components UpdateSprite2D
         */
        static void systemUpdateSprite2D(UpdateSprite2D &update, const Sprite2D &s);

    private:
        std::vector<Property> _update;
        static const std::vector<Property> _default;
    };
} // namespace RType

#endif /* !UPDATESPRITE2D_HPP_ */
