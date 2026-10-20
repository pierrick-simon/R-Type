/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Position2D
*/

#ifndef POSITION2D_HPP_
#define POSITION2D_HPP_

namespace RType {
    /**
     * @brief Position of an entity in 2D space, in pixels.
     *
     * The coordinate system follows raylib's: (0, 0) is the top-left corner,
     * x grows to the right and y grows downward.
     */
    class Position2D {
    public:
        /**
         * @brief Default constructor
         */
        Position2D(float x = 0, float y = 0) :
            _x(x), _y(y) {};

        /** @return The x coordinate (read only, does not flag an update). */
        [[nodiscard]] float x() const
        {
            return _x;
        }

        /** @return The y coordinate (read only, does not flag an update). */
        [[nodiscard]] float y() const
        {
            return _y;
        }

        /** @return A reference to x. Flags the position as updated. */
        [[nodiscard]] float &x()
        {
            _update = true;
            return _x;
        }

        /** @return A reference to y. Flags the position as updated. */
        [[nodiscard]] float &y()
        {
            _update = true;
            return _y;
        }

        /** @return true if the position changed since the last
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

    private:
        float _x;
        float _y;
        bool _update = true;
    };
}; // namespace RType

#endif /* !POSITION2D_HPP_ */
