/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Camera3D
*/

#include "Camera3D.hpp"
#include "Exceptions.hpp"

namespace RType {
    Camera3D::Camera3D(raylib::Vector3 target, raylib::Vector3 up, float fovy,
        int projection, int updateMode) :
        updateMode(updateMode)
    {
        static bool onlyOne = true;
        if (!onlyOne)
            throw EngineException("Camera can be created only once");
        onlyOne = false;
        this->target = target;
        this->up = up;
        this->fovy = fovy;
        this->projection = projection;
    }

    void Camera3D::systemUpdateCamera(Camera3D &camera, const Position3D &pos)
    {
        camera.SetPosition({pos.x, pos.y, pos.z});
        UpdateCamera(&camera, camera.updateMode);
    }
} // namespace RType
