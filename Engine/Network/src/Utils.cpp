/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Utils.cpp
*/

#include "Network.hpp"

namespace RType {

    static inline void put16(uint8_t *p, uint16_t v)
    {
        p[0] = static_cast<uint8_t>(v >> 8);
        p[1] = static_cast<uint8_t>(v);
    }

    static inline void put32(uint8_t *p, uint32_t v)
    {
        p[0] = static_cast<uint8_t>(v >> 24);
        p[1] = static_cast<uint8_t>(v >> 16);
        p[2] = static_cast<uint8_t>(v >> 8);
        p[3] = static_cast<uint8_t>(v);
    }

    static inline uint16_t get16(const uint8_t *p)
    {
        return static_cast<uint16_t>((p[0] << 8) | p[1]);
    }

    static inline uint32_t get32(const uint8_t *p)
    {
        return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
            (uint32_t(p[2]) << 8) | uint32_t(p[3]);
    }

    std::vector<uint8_t> serialize(
        Header h, const std::vector<uint8_t> &payload)
    {
        std::vector<uint8_t> out(Header::SIZE + payload.size());

        h.payloadSize = static_cast<uint16_t>(payload.size());
        out[0] = static_cast<uint8_t>(h.type);
        put16(&out[1], h.payloadSize);
        put32(&out[3], h.sequence);
        put16(&out[7], h.senderId);
        std::copy(payload.begin(), payload.end(), out.begin() + Header::SIZE);
        return out;
    }

    std::optional<Header> parse(
        const std::vector<uint8_t> &buf, std::size_t len)
    {
        Header h = {};

        if (len < Header::SIZE)
            return std::nullopt;
        h.type = static_cast<Opcode>(buf[0]);
        h.payloadSize = get16(buf.data() + 1);
        h.sequence = get32(buf.data() + 3);
        h.senderId = get16(buf.data() + 7);
        if (h.payloadSize != len - Header::SIZE)
            return std::nullopt;
        return h;
    }

} // namespace RType
