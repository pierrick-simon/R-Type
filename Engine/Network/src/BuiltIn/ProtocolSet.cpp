/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolSet
*/

#include "ProtocolSet.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include "ProtocolHelper.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"

namespace RType {

    enum class Kind { F32, U8, I8, U16, U32, POINT16 };

    Kind kindOf(SetProperty p)
    {
        switch (p) {
            case SetProperty::SCALE:
            case SetProperty::FONT_SIZE:
            case SetProperty::ROTATION:
            case SetProperty::SPACING:
            case SetProperty::VOLUME:
            case SetProperty::PITCH:
                return Kind::F32;
            case SetProperty::FLIP:
            case SetProperty::VISIBLE:
            case SetProperty::LOOP:
                return Kind::U8;
            case SetProperty::LAYER:
                return Kind::I8;
            case SetProperty::FRAME:
            case SetProperty::FRAME_DURATION:
                return Kind::U16;
            case SetProperty::COLOR:
                return Kind::U32;
            case SetProperty::ORIGIN:
                return Kind::POINT16;
            default:
                return Kind::U8;
        }
    }

    raylib::Color toColor(uint32_t u)
    {
        return raylib::Color(static_cast<uint8_t>(u >> 24),
            static_cast<uint8_t>(u >> 16),
            static_cast<uint8_t>(u >> 8),
            static_cast<uint8_t>(u));
    }

    /** Applies `fn` to the component T of `e`, if it has one. */
    template<typename T, typename Fn>
    void with(GameManager &gm, Entity e, Fn fn)
    {
        if (gm.hasComponent<T>(e))
            fn(gm.getComponent<T>(e));
    }

    void apply(
        GameManager &gm, Entity e, SetProperty prop, const SetValue &value)
    {
        const float f =
            std::holds_alternative<float>(value) ? std::get<float>(value) : 0.f;
        const int64_t i = std::holds_alternative<int64_t>(value)
            ? std::get<int64_t>(value)
            : 0;
        const Point16 pt = std::holds_alternative<Point16>(value)
            ? std::get<Point16>(value)
            : Point16 {0, 0};

        switch (prop) {
            case SetProperty::SCALE:
                with<Sprite2D>(gm, e, [f](auto &c) { c.scale = f; });
                with<SpriteSheet2D>(gm, e, [f](auto &c) { c.scale = f; });
                break;
            case SetProperty::FONT_SIZE:
                if (f > 0)
                    with<Text2D>(gm, e, [f](auto &c) { c.fontSize = f; });
                break;
            case SetProperty::ROTATION:
                with<Sprite2D>(gm, e, [f](auto &c) { c.rotation = f; });
                with<SpriteSheet2D>(gm, e, [f](auto &c) { c.rotation = f; });
                with<Text2D>(gm, e, [f](auto &c) { c.rotation = f; });
                break;
            case SetProperty::FLIP:
                with<Sprite2D>(gm, e, [i](auto &c) { c.flip = i != 0; });
                with<SpriteSheet2D>(gm, e, [i](auto &c) { c.flip = i != 0; });
                break;
            case SetProperty::COLOR: {
                const auto color = toColor(static_cast<uint32_t>(i));
                with<Sprite2D>(gm, e, [color](auto &c) { c.color = color; });
                with<SpriteSheet2D>(
                    gm, e, [color](auto &c) { c.color = color; });
                with<Text2D>(gm, e, [color](auto &c) { c.color = color; });
                break;
            }
            case SetProperty::ORIGIN:
                with<Sprite2D>(gm, e, [pt](auto &c) {
                    c.origin = {static_cast<float>(pt.first),
                        static_cast<float>(pt.second)};
                });
                with<SpriteSheet2D>(gm, e, [pt](auto &c) {
                    c.origin = {static_cast<float>(pt.first),
                        static_cast<float>(pt.second)};
                });
                with<Text2D>(gm, e, [pt](auto &c) {
                    c.origin = {static_cast<float>(pt.first),
                        static_cast<float>(pt.second)};
                });
                break;
            case SetProperty::FRAME:
                with<SpriteSheet2D>(gm, e, [i](auto &c) {
                    c.currentTile = static_cast<std::size_t>(i);
                });
                break;
            case SetProperty::FRAME_DURATION:
                with<SpriteSheet2D>(gm, e, [i](auto &c) {
                    c.frameDuration = std::chrono::milliseconds(i);
                });
                break;
            case SetProperty::SPACING:
                with<Text2D>(gm, e, [f](auto &c) { c.spacing = f; });
                break;
            case SetProperty::VOLUME:
                with<Sound>(gm, e, [f](auto &c) {
                    c.SetVolume(std::clamp(f, 0.f, 1.f));
                });
                break;
            case SetProperty::PITCH:
                if (f > 0)
                    with<Sound>(gm, e, [f](auto &c) { c.SetPitch(f); });
                break;
            case SetProperty::LOOP:
                with<Sound>(gm, e, [i](auto &c) { c.loop = i != 0; });
                break;
            case SetProperty::LAYER:
            case SetProperty::VISIBLE:
                // TODO: the engine has no Layer / Visible component yet.
                break;
            default:
                break;
        }
    }

    std::size_t ProtocolSet::valueSize(SetProperty prop)
    {
        switch (kindOf(prop)) {
            case Kind::F32:
            case Kind::U32:
            case Kind::POINT16:
                return 4;
            case Kind::U16:
                return 2;
            default:
                return 1;
        }
    }

    void ProtocolSet::manageSet(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        auto p = parseSet(payload);
        if (!p)
            return;
        auto entity = findNetEntity(ctx, p->id);
        if (!entity)
            return;
        if (auto *f = std::get_if<float>(&p->value); f && !std::isfinite(*f))
            return;
        apply(ctx.gm->get(), *entity, p->prop, p->value);
    }

    void ProtocolSet::sendSet(ServerManager &s, const SetPayload &p)
    {
        s.broadcast(Opcode::SET, serializeSet(p));
    }

    void ProtocolSet::sendSet(
        ServerManager &s, uint32_t id, SetProperty prop, SetValue value)
    {
        sendSet(s, SetPayload {id, prop, std::move(value)});
    }

    std::vector<uint8_t> ProtocolSet::serializeSet(const SetPayload &e)
    {
        std::vector<uint8_t> out(SetPayload::HEADER_SIZE + valueSize(e.prop));
        uint8_t *v = out.data() + SetPayload::HEADER_SIZE;
        const float f = std::holds_alternative<float>(e.value)
            ? std::get<float>(e.value)
            : 0.f;
        const int64_t i = std::holds_alternative<int64_t>(e.value)
            ? std::get<int64_t>(e.value)
            : 0;
        const Point16 pt = std::holds_alternative<Point16>(e.value)
            ? std::get<Point16>(e.value)
            : Point16 {0, 0};

        put32(&out[0], e.id);
        out[4] = static_cast<uint8_t>(e.prop);
        switch (kindOf(e.prop)) {
            case Kind::F32:
                put32(v, std::bit_cast<uint32_t>(f));
                break;
            case Kind::U8:
            case Kind::I8:
                v[0] = static_cast<uint8_t>(i);
                break;
            case Kind::U16:
                put16(v, static_cast<uint16_t>(i));
                break;
            case Kind::U32:
                put32(v, static_cast<uint32_t>(i));
                break;
            case Kind::POINT16:
                put16(v, static_cast<uint16_t>(pt.first));
                put16(v + 2, static_cast<uint16_t>(pt.second));
                break;
        }
        return out;
    }

    std::optional<SetPayload> ProtocolSet::parseSet(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() <= SetPayload::HEADER_SIZE ||
            payload[4] >= static_cast<uint8_t>(SetProperty::Count))
            return std::nullopt;
        SetPayload e {};
        e.id = get32(payload.data());
        e.prop = static_cast<SetProperty>(payload[4]);
        if (payload.size() != SetPayload::HEADER_SIZE + valueSize(e.prop))
            return std::nullopt;
        const uint8_t *v = payload.data() + SetPayload::HEADER_SIZE;
        switch (kindOf(e.prop)) {
            case Kind::F32:
                e.value = std::bit_cast<float>(get32(v));
                break;
            case Kind::U8:
                e.value = static_cast<int64_t>(v[0]);
                break;
            case Kind::I8:
                e.value = static_cast<int64_t>(static_cast<int8_t>(v[0]));
                break;
            case Kind::U16:
                e.value = static_cast<int64_t>(get16(v));
                break;
            case Kind::U32:
                e.value = static_cast<int64_t>(get32(v));
                break;
            case Kind::POINT16:
                e.value = Point16 {static_cast<int16_t>(get16(v)),
                    static_cast<int16_t>(get16(v + 2))};
                break;
        }
        return e;
    }

} // namespace RType
