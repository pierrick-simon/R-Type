/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** WalkerDef.hpp
*/

#ifndef WALKERDEF_HPP_
    #define WALKERDEF_HPP_
    #include "DimensionsDef.hpp"

namespace RType {

    struct WalkerDef {
        float stepHeight;
        float crouchTime;
        float turnDelay;
        float jumpApex;
        float gravity;
        float landingDistance;
    };


    inline constexpr WalkerDef WALKER_DEF = {.stepHeight = 45.f,
        .crouchTime = 0.25f,
        .turnDelay = 0.3f,
        .jumpApex = 140.f,
        .gravity = 2500.f,
        .landingDistance = 0.8f};

} // namespace RType

#endif
