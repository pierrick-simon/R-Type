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
        /**
         * @brief System to update the components UpdatePosition2D
         */
        static void systemUpdatePosition2D(
            UpdatePosition2D &update, const Position2D &pos);

    private:
        bool update = true;
    };
} // namespace RType

#endif /* !UPDATEPOSITION2D_HPP_ */
