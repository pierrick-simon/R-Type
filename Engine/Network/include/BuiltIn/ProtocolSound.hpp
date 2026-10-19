/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ProtocolSound
*/

#ifndef PROTOCOL_SOUND_HPP_
    #define PROTOCOL_SOUND_HPP_

#include <optional>
#include <vector>
#include "GameManager.hpp"
#include "Network.hpp"
#include "ServerManager.hpp"

namespace RType {

    /**
     * @brief Payload shared by PLAY, PAUSE and STOP (server -> client).
     *
     * Layout: entityId (4).
     */
    struct SoundPayload {
        static constexpr std::size_t SIZE = 4;
        uint32_t id;
    };

    class ProtocolSound {
    public:
        ProtocolSound() = default;

        /** @brief Sound: play (resume if paused). SpriteSheet: start the
         * animation. */
        static void managePlay(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        /** @brief Sound: pause. SpriteSheet: freeze the animation. */
        static void managePause(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        /** @brief Sound: stop. SpriteSheet: stop and reset to frame 0. */
        static void manageStop(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);

        /** @brief Server helpers: broadcast the matching command. */
        static void sendPlay(ServerManager &s, uint32_t id);
        static void sendPause(ServerManager &s, uint32_t id);
        static void sendStop(ServerManager &s, uint32_t id);

        static std::vector<uint8_t> serializeSound(const SoundPayload &e);
        static std::optional<SoundPayload> parseSound(
            const std::vector<uint8_t> &payload);
    };

} // namespace RType

#endif /* !PROTOCOL_SOUND_HPP_ */
