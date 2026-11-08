/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CameraAngle
*/

#include "CameraAngle.hpp"
#include <algorithm>
#include <cmath>

namespace RType {

    void CameraAngle::rotate(float dYaw, float dPitch)
    {
        constexpr float twoPi = 2.f * MY_PI;

        yaw = std::fmod(yaw + dYaw, twoPi);
        if (yaw >= MY_PI)
            yaw -= twoPi;
        else if (yaw < -MY_PI)
            yaw += twoPi;
        pitch = std::clamp(pitch + dPitch, minPitch, maxPitch);
    }

    raylib::Vector3 CameraAngle::forward() const
    {
        const float cp = std::cos(pitch);

        return raylib::Vector3 {
            std::sin(yaw) * cp, std::sin(pitch), -std::cos(yaw) * cp};
    }

    void CameraAngle::systemCameraAngleInput(
        CameraAngle &angle, CameraAngleInput &input)
    {
        angle.rotate(input.yaw, input.pitch);
        input.yaw = 0.f;
        input.pitch = 0.f;
    }

    void CameraAngle::systemCameraAngle(
        Camera3D &camera, const Position3D &pos, const CameraAngle &angle)
    {
        const auto f = angle.forward();

        camera.SetTarget({pos.x + f.x, pos.y + f.y, pos.z + f.z});
    }
} // namespace RType
