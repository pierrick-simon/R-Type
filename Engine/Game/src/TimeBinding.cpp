/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** TimeBinding
*/

#include "TimeBinding.hpp"
#include "Clock.hpp"
#include "GameManagerBinding.hpp"

namespace RType::Game {

    void TimeBinding::registerFunctions(sol::state &lua)
    {
        lua["GameManager"]["elapsed"] = [](GameManager &gm) {
            return GameManagerBinding::findModuleComponent<MasterClock>(
                gm, "gm:elapsed", "Clock")
                .getElapsed()
                .count();
        };
    }

} // namespace RType::Game
