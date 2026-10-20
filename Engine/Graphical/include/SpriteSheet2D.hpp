/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SpriteSheet2D
*/

#ifndef SPRITESHEET2D_HPP_
#define SPRITESHEET2D_HPP_

#include <optional>
#include "Clock.hpp"
#include "GameManager.hpp"
#include "Origin2D.hpp"
#include "Position2D.hpp"
#include "Property.hpp"
#include "Size2D.hpp"
#include "Texture2D.hpp"

namespace RType {

    /**
     * @brief Component describing an animated sprite sheet.
     *
     * The texture is split into a grid of tiles (recNb()); the system displays
     * one tile at a time and advances to the next one every frameDuration().
     *
     * @note The systems write directly to the members, so the local animation
     *       (currentTile advancing each frame) is NOT flagged as an update.
     *       Only changes made through the accessors are.
     */
    class SpriteSheet2D {
    public:
        /** @brief SpriteSheet2D default constructor
         */
        SpriteSheet2D() = default;
        SpriteSheet2D(Size2D recNb, unsigned int frameDuration, float scale) :
            _recNb(recNb), _frameDuration(frameDuration), _scale(scale) {};
        SpriteSheet2D(Size2D recNb, std::size_t currentTile,
            unsigned int frameDuration, float scale, bool animation = true) :
            _recNb(recNb),
            _currentTile(currentTile),
            _frameDuration(frameDuration),
            _scale(scale),
            _animation(animation) {};

        /**
         * @brief Number of tiles in the texture (not synchronized).
         *
         * @warning Members of Size2D are ordered `{height, width}`: `{1, 1}`
         *          means a single tile. Make sure to give the number of rows
         *          first, then the number of columns.
         * @warning Must not contain 0 (division by zero when computing the
         *          tile size).
         */
        [[nodiscard]] const Size2D &recNb() const
        {
            return _recNb;
        }

        [[nodiscard]] Size2D &recNb()
        {
            return _recNb;
        }

        /** @brief Index of the displayed tile, row by row. Flags FRAME. */
        [[nodiscard]] std::size_t currentTile() const
        {
            return _currentTile;
        }

        [[nodiscard]] std::size_t &currentTile()
        {
            _update.emplace_back(Property::FRAME);
            return _currentTile;
        }

        /** @brief Time spent on each tile. Flags FRAME_DURATION. */
        [[nodiscard]] const std::chrono::milliseconds &frameDuration() const
        {
            return _frameDuration;
        }

        [[nodiscard]] std::chrono::milliseconds &frameDuration()
        {
            _update.emplace_back(Property::FRAME_DURATION);
            return _frameDuration;
        }

        /** @brief Time elapsed on the current tile (not synchronized). */
        [[nodiscard]] const std::chrono::milliseconds &sleep() const
        {
            return _sleep;
        }

        [[nodiscard]] std::chrono::milliseconds &sleep()
        {
            return _sleep;
        }

        /** @brief Origin of the tile (rotation pivot). Flags ORIGIN. */
        [[nodiscard]] const Origin2D &origin() const
        {
            return _origin;
        }

        [[nodiscard]] Origin2D &origin()
        {
            _update.emplace_back(Property::ORIGIN);
            return _origin;
        }

        /** @brief Whether the animation is running (not synchronized). */
        [[nodiscard]] bool animation() const
        {
            return _animation;
        }

        [[nodiscard]] bool &animation()
        {
            return _animation;
        }

        /** @brief Scale factor applied to the tile. Flags SCALE. */
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

        /**
         * @brief System drawing a sprite sheet.
         *
         * @param sprite  The sprite sheet parameters and animation state.
         * @param texture The texture containing all the tiles.
         * @param pos     The position where the current tile is drawn.
         *
         * @warning Must be called between the beginning and the end of a
         *          drawing frame (see Window::systemWindow).
         * @warning recNb() must not contain 0 (it would lead to a division by
         *          zero when computing the tile size).
         */
        static void systemDrawSpriteSheet2D(SpriteSheet2D &sprite,
            const Texture2D &texture, const Position2D &pos);

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
         * @brief System animating a sprite sheet.
         *
         * @param sprite  The sprite sheet parameters and animation state.
         * @param clock   The clock providing the elapsed time.
         *
         * @warning Clock::systemClock must have been called beforehand in the
         *          same frame, otherwise the animation will not progress.
         */
        static void systemAnimateSpriteSheet2D(
            SpriteSheet2D &sprite, const Clock &clock);
        static void systemMasterAnimateSpriteSheet2D(
            GameManager &gm, const MasterClock &clock);

    private:
        Size2D _recNb = {1, 1};
        std::size_t _currentTile = 0;
        std::chrono::milliseconds _frameDuration =
            std::chrono::milliseconds(100);
        std::chrono::milliseconds _sleep = std::chrono::milliseconds(0);
        Origin2D _origin;
        bool _animation = true;
        float _scale = 1;
        float _rotation = 0;
        bool _flip = false;
        raylib::Color _color = raylib::Color::White();

        std::vector<Property> _update;
        static const std::vector<Property> _default;
    };

} // namespace RType

#endif /* !SPRITESHEET2D_HPP_ */
