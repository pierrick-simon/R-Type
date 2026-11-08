/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ScoutDef.hpp
*/

#ifndef SCOUTDEF_HPP_
    #define SCOUTDEF_HPP_
    #include <array>
    #include "DimensionsDef.hpp"

namespace RType {

    struct ScoutZone {
        float start;
        float end;
    };

    struct ScoutDef {
        float minInterval;
        float maxInterval;
        int minGroup;
        int maxGroup;
        float spacing;
        float spread;
        float minSwingSpeed;
        float maxSwingSpeed;
        float minSwingTime;
        float maxSwingTime;
        float speedJitter;
        int spawnTries;
    };

    inline constexpr std::array<ScoutZone, 2> SCOUT_ZONES = {{
        {.start = 560.f, .end = 1460.f},
        {.start = 3520.f, .end = 4032.f},
    }};

    inline constexpr ScoutDef SCOUT_DEF = {.minInterval = 5.f,
        .maxInterval = 10.f,
        .minGroup = 3,
        .maxGroup = 10,
        .spacing = 110.f,
        .spread = 150.f,
        .minSwingSpeed = 70.f,
        .maxSwingSpeed = 160.f,
        .minSwingTime = 0.3f,
        .maxSwingTime = 0.7f,
        .speedJitter = 0.12f,
        .spawnTries = 12};

} // namespace RType

#endif
