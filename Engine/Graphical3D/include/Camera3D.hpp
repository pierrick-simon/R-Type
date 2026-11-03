/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Camera3D
*/

#ifndef CAMERA3D_HPP_
#define CAMERA3D_HPP_

#include <raylib-cpp.hpp>
#include "Position3D.hpp"

namespace RType {
    /**
     * @brief 3D camera component built on top of raylib::Camera.
     *
     * Holds the raylib camera parameters along with the update mode used
     * when the camera is refreshed each frame.
     */
    struct Camera3D : raylib::Camera {
        /**
         * @brief Constructs a Camera3D.
         * @param target Point the camera looks at.
         * @param up Up vector of the camera.
         * @param fovy Vertical field of view (degrees in perspective mode).
         * @param projection Projection type (e.g. CAMERA_PERSPECTIVE).
         * @param updateMode Camera update mode (e.g. CAMERA_CUSTOM).
         */
        Camera3D(raylib::Vector3 target = raylib::Vector3 {0.f, 0.4f, 0.f},
            raylib::Vector3 up = raylib::Vector3 {0.f, 1.f, 0.f},
            float fovy = 45.f, int projection = CAMERA_PERSPECTIVE,
            int updateMode = CAMERA_CUSTOM);

        int updateMode;

        /**
         * @brief System updating a camera from a position.
         * @param camera Camera to update.
         * @param pos Position component used to place the camera.
         */
        static void systemUpdateCamera(Camera3D &camera, const Position3D &pos);
    };
} // namespace RType

#endif /* !CAMERA3D_HPP_ */
