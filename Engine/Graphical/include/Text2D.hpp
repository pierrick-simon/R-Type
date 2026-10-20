/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Text2d
*/

#ifndef TEXT2D_HPP_
#define TEXT2D_HPP_

#include <optional>
#include <raylib-cpp.hpp>
#include <string>
#include "Font.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Property.hpp"

namespace RType {

    /**
     * @brief Text component drawn on screen at a given position.
     */
    class Text2D {
    public:
        /** @brief Size of the font in pixels. Flags FONT_SIZE. */
        [[nodiscard]] float fontSize() const
        {
            return _fontSize;
        }

        [[nodiscard]] float &fontSize()
        {
            _update.emplace_back(Property::FONT_SIZE);
            return _fontSize;
        }

        /** @brief Displayed string. */
        [[nodiscard]] const std::string &str() const
        {
            return _str;
        }

        [[nodiscard]] std::string &str()
        {
            _updateText = true;
            return _str;
        }

        /** @brief Text color. Flags COLOR. */
        [[nodiscard]] const raylib::Color &color() const
        {
            return _color;
        }

        [[nodiscard]] raylib::Color &color()
        {
            _update.emplace_back(Property::COLOR);
            return _color;
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

        /** @brief Spacing between characters, in pixels. Flags SPACING. */
        [[nodiscard]] float spacing() const
        {
            return _spacing;
        }

        [[nodiscard]] float &spacing()
        {
            _update.emplace_back(Property::SPACING);
            return _spacing;
        }

        /** @brief Origin of the text (rotation pivot). Flags ORIGIN. */
        [[nodiscard]] const Origin2D &origin() const
        {
            return _origin;
        }

        [[nodiscard]] Origin2D &origin()
        {
            _update.emplace_back(Property::ORIGIN);
            return _origin;
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

        /** @return true if the text changed since the last
         * setUpdate(false). */
        [[nodiscard]] bool getUpdateText() const
        {
            return _updateText;
        }

        /**
         * @brief Manually set the update flag.
         *
         * @param update New value of the flag (false once the change was sent).
         */
        void setUpdateText(bool update = true)
        {
            _updateText = update;
        }

        /**
         * @brief System drawing a text.
         *
         * @param text The text component to draw.
         * @param font The font used to render the text.
         * @param pos  The position where the text is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning @p font must stay valid for the whole call.
         */
        static void systemDrawText2D(
            const Text2D &text, const Font &font, const Position2D &pos);

    private:
        float _fontSize = 10;
        std::string _str = "Hello World";
        raylib::Color _color = raylib::Color::White();
        float _rotation = 0;
        float _spacing = 0;
        Origin2D _origin;

        std::vector<Property> _update;
        bool _updateText = true;
        static const std::vector<Property> _default;
    };

} // namespace RType

#endif /* !TEXT2D_HPP_ */
