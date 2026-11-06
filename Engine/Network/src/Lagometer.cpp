/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Lagometer
*/

#include "Lagometer.hpp"
#include "Network.hpp"

namespace RType {
    void Lagometer::start()
    {
        _clock = std::chrono::steady_clock::now();
    }

    void Lagometer::stop()
    {
        auto now = std::chrono::steady_clock::now();
        auto _ping =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - _clock);
        if (_ping.count() == 0)
            _ping = std::chrono::milliseconds(1);
        _info.ping = float(_ping.count());
    }

    void Lagometer::reset()
    {
        _recieved = 0;
        _sent = 0;
    }

    void Lagometer::packetRecieved()
    {
        _recieved++;
    }

    void Lagometer::packetSent()
    {
        _sent++;
    }

    void Lagometer::stat(unsigned int beenSent, unsigned int beenRecieved)
    {
        _info.lastBeenRecieved = beenRecieved;
        _info.lastBeenSent = beenSent;
        _info.lastRecieved = _recieved;
        _info.lastSent = _sent;
        reset();
    }

    std::vector<uint8_t> Lagometer::serializeLagometer(
        const LagometerPayload &e)
    {
        std::vector<uint8_t> out(LagometerPayload::SIZE);

        put32(&out[0], e.beenRecieved);
        put32(&out[4], e.beenSent);
        return out;
    }

    std::optional<LagometerPayload> Lagometer::parseLagometer(
        const std::vector<uint8_t> &payload)
    {
        if (payload.size() != LagometerPayload::SIZE)
            return std::nullopt;
        LagometerPayload e {};
        e.beenRecieved = get32(payload.data());
        e.beenSent = get32(payload.data() + 4);
        return e;
    }
} // namespace RType
