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
    struct Camera3D : raylib::Camera {
        Camera3D(raylib::Vector3 target = raylib::Vector3 {0.f, 0.4f, 0.f},
            raylib::Vector3 up = raylib::Vector3 {0.f, 1.f, 0.f},
            float fovy = 45.f, int projection = CAMERA_PERSPECTIVE,
            int updateMode = CAMERA_CUSTOM);

        int updateMode;

        static void systemUpdateCamera(Camera3D &camera, const Position3D &pos);
    };
} // namespace RType

#endif /* !CAMERA3D_HPP_ */
