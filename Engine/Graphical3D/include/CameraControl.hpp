/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CameraControl
*/

#ifndef CAMERACONTROL_HPP_
#define CAMERACONTROL_HPP_

#include "GameManager.hpp"
#include "MouseManager.hpp"

namespace RType {

    /**
     * @brief Turns the mouse movement into a CameraAngleInput.
     */
    struct CameraMouseControl {
        float sensitivity = 0.0025f;
        bool invertY = false;

        static void systemCameraMouseControl(
            GameManager &gm, const MouseManager &mouse);
    };

} // namespace RType

#endif /* !CAMERACONTROL_HPP_ */
