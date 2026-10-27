
/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Property
*/

#ifndef PROPERTY_HPP_
#define PROPERTY_HPP_

#include <cstdint>
#include <tuple>
#include <variant>

namespace RType {

    /**
     * @brief Property Of a Components.
     */
    enum class Property : uint8_t {
        SCALE = 0, //!< f32
        FONT_SIZE = 1, //!< f32
        ROTATION = 2, //!< f32, degrees
        FLIP = 3, //!< u8
        COLOR = 4, //!< u32 RGBA
        LAYER = 5, //!< i8
        VISIBLE = 6, //!< u8
        ORIGIN = 7, //!< i16 x, i16 y
        FRAME = 8, //!< u16 (currentTile)
        FRAME_DURATION = 9, //!< u16, ms
        SPACING = 10, //!< f32
        VOLUME = 11, //!< f32, 0 to 1
        PITCH = 12, //!< f32
        LOOP = 13, //!< u8
        REC_NB = 14, //!< i16 width, i16 height
        Count
    };

    /** @brief Value of a packet: f32, integer (u8/i8/u16/u32), or i16 pair.
     */
    using Point16 = std::pair<int16_t, int16_t>;
    using SetValue = std::variant<float, int64_t, Point16>;

} // namespace RType

#endif /* !PROPERTY_HPP_ */
