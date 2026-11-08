/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CameraControl
*/

#include "CameraControl.hpp"
#include "CameraAngle.hpp"

namespace RType {

    void CameraMouseControl::systemCameraMouseControl(
        GameManager &gm, const MouseManager &mouse)
    {
        for (auto [_, input, control] :
            gm.view<CameraAngleInput, CameraMouseControl>().getVector()) {

            const auto delta = mouse.getDelta();
            const float ySign = control.invertY ? 1.f : -1.f;

            input.yaw += delta.x * control.sensitivity;
            input.pitch += ySign * delta.y * control.sensitivity;
        }
    }
} // namespace RType
