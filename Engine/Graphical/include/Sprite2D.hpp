/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Sprite2D
*/

#ifndef SPRITE2D_HPP_
#define SPRITE2D_HPP_

#include <optional>
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Property.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing how a whole texture is drawn.
     *
     * @note On a non-const object, even a read through the overloaded accessor
     *       flags the property. Use a const reference to read without flagging.
     */
    class Sprite2D {
    public:
        /** @brief Origin of the sprite (rotation pivot). Flags ORIGIN. */
        [[nodiscard]] const Origin2D &origin() const
        {
            return _origin;
        }

        [[nodiscard]] Origin2D &origin()
        {
            _update.emplace_back(Property::ORIGIN);
            return _origin;
        }

        /** @brief Scale factor applied to the texture. Flags SCALE. */
        [[nodiscard]] float scale() const
        {
            return _scale;
        }

        [[nodiscard]] float &scale()
        {
            _update.emplace_back(Property::SCALE);
            return _scale;
        }

        /** @brief Rotation in degrees. Flags ROTATION. */
        [[nodiscard]] float rotation() const
        {
            return _rotation;
        }

        [[nodiscard]] float &rotation()
        {
            _update.emplace_back(Property::ROTATION);
            return _rotation;
        }

        /** @brief Horizontal flip. Flags FLIP. */
        [[nodiscard]] bool flip() const
        {
            return _flip;
        }

        [[nodiscard]] bool &flip()
        {
            _update.emplace_back(Property::FLIP);
            return _flip;
        }

        /** @brief Tint color. Flags COLOR. */
        [[nodiscard]] const raylib::Color &color() const
        {
            return _color;
        }

        [[nodiscard]] raylib::Color &color()
        {
            _update.emplace_back(Property::COLOR);
            return _color;
        }

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
        [[nodiscard]] static const std::vector<Property> &getDefaultUpdate();

        /** @brief Clear update
         */
        void clearUpdate()
        {
            _update.clear();
        }

        /**
         * @brief System drawing a sprite.
         *
         * @param sprite  The sprite parameters (scale, rotation, tint...).
         * @param texture The texture to draw.
         * @param pos     The position where the sprite is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning @p texture must stay valid for the whole call.
         */

        static void systemDrawSprite2D(const Sprite2D &sprite,
            const Texture2D &texture, const Position2D &pos);

    private:
        Origin2D _origin;
        float _scale = 1;
        float _rotation = 0;
        bool _flip = false;
        raylib::Color _color = raylib::Color::White();

        std::vector<Property> _update;
        static const std::vector<Property> _default;
    };

} // namespace RType

#endif /* !SPRITE2D_HPP_ */
