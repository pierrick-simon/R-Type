/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AudioBinding
*/

#include "AudioBinding.hpp"
#include "GameManagerBinding.hpp"
#include "Listener.hpp"

namespace RType::Game {

    void AudioBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["setListenerPosition"] =
            [](GameManager &gm, float x, float y) {
                Listener::systemListener2D(
                    GameManagerBinding::findModuleComponent<Listener>(
                        gm, "gm:setListenerPosition", "Audio"),
                    Position2D {x, y});
            };
    }

} // namespace RType::Game
