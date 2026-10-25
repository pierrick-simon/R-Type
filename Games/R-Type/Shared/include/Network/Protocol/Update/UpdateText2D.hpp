/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdateText2D
*/

#ifndef UPDATETEXT2D_HPP_
#define UPDATETEXT2D_HPP_

#include "Property.hpp"
#include "Text2D.hpp"

namespace RType {
    class UpdateText2D : public Text2D {
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
         * @brief System to update the components UpdateText2D
         */
        static void systemUpdateText2D(
            UpdateText2D &update, const Text2D &text);

    private:
        std::vector<Property> _update;
        bool _updateText = true;
        static const std::vector<Property> _default;
    };
}; // namespace RType

#endif /* !UPDATETEXT2D_HPP_ */
