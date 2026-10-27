/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UpdatePosition2D
*/

#ifndef UPDATEPOSITION2D_HPP_
#define UPDATEPOSITION2D_HPP_

#include "Position2D.hpp"

namespace RType {
    class UpdatePosition2D : public Position2D {
    public:
        /** @return true if the pos changed since the last
         * setUpdate(false). */
        [[nodiscard]] bool getUpdate() const
        {
            return _update;
        }

        /**
         * @brief Manually set the update flag.
         *
         * @param update New value of the flag (false once the change was sent).
         */
        void setUpdate(bool update = true)
        {
            _update = update;
        }

        /**
         * @brief System to update the components UpdatePosition2D
         */
        static void systemUpdatePosition2D(
            UpdatePosition2D &update, const Position2D &pos);

    private:
        bool _update = true;
    };
} // namespace RType

#endif /* !UPDATEPOSITION2D_HPP_ */
