/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** CameraAngle
*/

#ifndef CAMERAANGLE_HPP_
#define CAMERAANGLE_HPP_

#include <raylib-cpp.hpp>
#include "Camera3D.hpp"
#include "Clock.hpp"
#include "GameManager.hpp"
#include "Position3D.hpp"

namespace RType {

    /**
     * @brief Angle delta (radians) to apply to a CameraAngle this frame.
     *
     * Input producers (mouse, keyboard, gamepad, network...) ADD to these
     * values. systemCameraAngleInput() consumes them and resets them to 0.
     * Positive yaw turns right, positive pitch looks up.
     */
    struct CameraAngleInput {
        float yaw = 0.f;
        float pitch = 0.f;
    };

    /**
     * @brief Orientation of a 3D camera, in radians.
     *
     * Right-handed, y up. yaw = 0 and pitch = 0 look toward -Z.
     * Positive yaw turns toward +X (right), positive pitch looks up.
     * Pitch is clamped to [minPitch, maxPitch] to avoid flipping over.
     */
    class CameraAngle {
    public:
        static constexpr float MY_PI = 3.14159265358979323846f;

        float yaw = 0.f;
        float pitch = 0.f;
        float minPitch = -89.f * MY_PI / 180.f;
        float maxPitch = 89.f * MY_PI / 180.f;

        /**
         * @brief Consume the input delta and rotate the angle.
         * @param angle The angle to update.
         * @param input The input, reset to 0 afterwards.
         */
        static void systemCameraAngleInput(
            CameraAngle &angle, CameraAngleInput &input);

        /**
         * @brief Point the camera along the angle, from its position.
         *
         * Sets camera.target = pos + forward. The position itself is still
         * set by Camera3D::systemUpdateCamera().
         *
         * @warning Use updateMode CAMERA_CUSTOM: raylib's built-in modes
         *          (FIRST_PERSON, ORBITAL...) would override the target.
         */
        static void systemCameraAngle(
            Camera3D &camera, const Position3D &pos, const CameraAngle &angle);

    private:
        /// Add a delta (radians), wrap the yaw and clamp the pitch.
        void rotate(float dYaw, float dPitch);

        /// Unit vector the camera looks along.
        [[nodiscard]] raylib::Vector3 forward() const;
    };

} // namespace RType

#endif /* !CAMERAANGLE_HPP_ */
