/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Listener
*/

#include "Listener.hpp"
#include "Exceptions.hpp"

namespace RType {
    Listener::Listener() :
        _engine(std::make_unique<ma_engine>())
    {
        static bool created = false;
        if (created)
            throw EngineException("Listener can be created only once");

        if (ma_engine_init(nullptr, _engine.get()) != MA_SUCCESS) {
            throw EngineException("Failed to initialise audio engine");
        }
        created = true;
    }

    void Listener::systemListener2D(Listener &l, const Position2D &pos)
    {
        ma_engine_listener_set_position(l._engine.get(), 0, pos.x, pos.y, 0.0f);
    }
} // namespace RType
