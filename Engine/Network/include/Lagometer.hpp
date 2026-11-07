/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lagometer
*/

#ifndef LAGOMETER_HPP_
#define LAGOMETER_HPP_

#include <chrono>

namespace RType {
    struct LagometerPayload {
        static constexpr unsigned int SIZE = 4 + 4;
        uint32_t beenRecieved;
        uint32_t beenSent;
    };

    class Lagometer {
    public:
        struct Info {
            float ping = 0.f;
            unsigned int lastSent = 0;
            unsigned int lastRecieved = 0;
            unsigned int lastBeenRecieved = 0;
            unsigned int lastBeenSent = 0;
        };

        void start();
        void stop();
        void reset();
        void packetRecieved();
        void packetSent();
        void stat(unsigned int beenSent, unsigned int beenRecieved);

        [[nodiscard]] unsigned int getSent() const
        {
            return _sent;
        }

        [[nodiscard]] unsigned int getRecieved() const
        {
            return _recieved;
        }

        [[nodiscard]] const Info &getInfo() const
        {
            return _info;
        }

        static std::vector<uint8_t> serializeLagometer(
            const LagometerPayload &e);
        static std::optional<LagometerPayload> parseLagometer(
            const std::vector<uint8_t> &payload);

    private:
        std::chrono::steady_clock::time_point _clock;
        std::chrono::milliseconds _ping = std::chrono::milliseconds(0);
        unsigned int _sent = 0;
        unsigned int _recieved = 0;
        Info _info = {};
    };
} // namespace RType

#endif /* !LAGOMETER_HPP_ */
