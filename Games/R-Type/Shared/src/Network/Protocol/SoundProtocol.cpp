/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** SoundProtocol
*/

#include "SoundProtocol.hpp"
#include "HelperProtocol.hpp"
#include "Sound.hpp"
#include "SpriteSheet2D.hpp"

namespace RType {

    void SoundProtocol::managePlay(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        auto e = parseSound(payload);
        if (!e)
            return;
        auto entity = findNetEntity(ctx, e->id);
        if (!entity)
            return;
        auto &gm = ctx.gm->get();

        if (gm.hasComponent<Sound>(*entity)) {
            auto &s = gm.getComponent<Sound>(*entity);
            s.state = s.state == Sound::State::PAUSED ? Sound::State::START
                                                      : Sound::State::PLAY;
        } else if (gm.hasComponent<SpriteSheet2D>(*entity)) {
            gm.getComponent<SpriteSheet2D>(*entity).animation = true;
        }
    }

    void SoundProtocol::managePause(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        auto e = parseSound(payload);
        if (!e)
            return;
        auto entity = findNetEntity(ctx, e->id);
        if (!entity)
            return;
        auto &gm = ctx.gm->get();

        if (gm.hasComponent<Sound>(*entity)) {
            auto &s = gm.getComponent<Sound>(*entity);
            if (s.state == Sound::State::PLAYING ||
                s.state == Sound::State::PLAY || s.state == Sound::State::START)
                s.state = Sound::State::PAUSED;
        } else if (gm.hasComponent<SpriteSheet2D>(*entity)) {
            gm.getComponent<SpriteSheet2D>(*entity).animation = false;
        }
    }

    void SoundProtocol::manageStop(
        const Header &, const std::vector<uint8_t> &payload, Context &ctx)
    {
        auto e = parseSound(payload);
        if (!e)
            return;
        auto entity = findNetEntity(ctx, e->id);
        if (!entity)
            return;
        auto &gm = ctx.gm->get();

        if (gm.hasComponent<Sound>(*entity)) {
            auto &s = gm.getComponent<Sound>(*entity);
            s.state = Sound::State::STOP;
        } else if (gm.hasComponent<SpriteSheet2D>(*entity)) {
            auto &sheet = gm.getComponent<SpriteSheet2D>(*entity);
            sheet.animation = false;
            sheet.currentTile = 0;
        }
    }

    void SoundProtocol::sendPlay(ServerManager &s, uint32_t id)
    {
        s.broadcast(OpcodeRType::PLAY, serializeSound({id}));
    }

    void SoundProtocol::sendPause(ServerManager &s, uint32_t id)
    {
        s.broadcast(OpcodeRType::PAUSE, serializeSound({id}));
    }

    void SoundProtocol::sendStop(ServerManager &s, uint32_t id)
    {
        s.broadcast(OpcodeRType::STOP, serializeSound({id}));
    }

    std::vector<uint8_t> SoundProtocol::serializeSound(const SoundPayload &e)
    {
        std::vector<uint8_t> out(SoundPayload::SIZE);

        put32(&out[0], e.id);
        return out;
    }

    std::optional<SoundPayload> SoundProtocol::parseSound(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != SoundPayload::SIZE)
            return std::nullopt;
        return SoundPayload {get32(payload.data())};
    }

} // namespace RType
