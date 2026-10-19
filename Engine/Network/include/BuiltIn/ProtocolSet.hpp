/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolSet
*/

#ifndef PROTOCOL_SET_HPP_
    #define PROTOCOL_SET_HPP_

#include <cstdint>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Property changed by a SET packet (PROTOCOL.md, section 5.3).
     *
     * The size of the value is fixed per property, see valueSize().
     */
    enum class SetProperty : uint8_t {
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
        Count
    };

    /** @brief Value of a SET packet: f32, integer (u8/i8/u16/u32), or i16 pair.
     */
    using Point16 = std::pair<int16_t, int16_t>;
    using SetValue = std::variant<float, int64_t, Point16>;

    /**
     * @brief Payload of a SET packet (server -> client).
     *
     * Layout: entityId (4) | prop (1) | value (valueSize(prop)).
     */
    struct SetPayload {
        static constexpr std::size_t HEADER_SIZE = 4 + 1;
        uint32_t id;
        SetProperty prop;
        SetValue value;
    };

    class ProtocolSet {
    public:
        ProtocolSet() = default;

        /** @brief Client handler: applies the property to the entity. */
        void manageSet(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helpers: broadcast one property change. */
        static void sendSet(ServerManager &s, const SetPayload &p);
        static void sendSet(
            ServerManager &s, uint32_t id, SetProperty prop, SetValue value);

        /** @return Size in bytes of the value of `prop`. */
        static std::size_t valueSize(SetProperty prop);

        static std::vector<uint8_t> serializeSet(const SetPayload &e);
        static std::optional<SetPayload> parseSet(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !PROTOCOL_SET_HPP_ */
