/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Rotation3D
*/

#ifndef ROTATION3D_HPP_
#define ROTATION3D_HPP_

#include <cmath>

namespace RType {

    /**
     * @brief Orientation of an entity in 3D space, as a unit quaternion.
     *
     * Follows raylib's 3D convention: right-handed, y is up.
     * Identity is (0, 0, 0, 1). A quaternion avoids gimbal lock, which
     * Euler angles suffer from.
     */
    struct Rotation3D {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
        float w = 1.f;

        static Rotation3D fromAxisAngle(
            float ax, float ay, float az, float angle)
        {
            float len = std::sqrt(ax * ax + ay * ay + az * az);
            if (len == 0.f)
                return Rotation3D {};
            float s = std::sin(angle * 0.5f) / len;
            return Rotation3D {ax * s, ay * s, az * s, std::cos(angle * 0.5f)};
        }

        static Rotation3D fromEuler(float pitch, float yaw, float roll)
        {
            return fromAxisAngle(0, 1, 0, yaw) * fromAxisAngle(1, 0, 0, pitch) *
                fromAxisAngle(0, 0, 1, roll);
        }

        Rotation3D operator*(const Rotation3D &o) const
        {
            return Rotation3D {
                w * o.x + x * o.w + y * o.z - z * o.y,
                w * o.y - x * o.z + y * o.w + z * o.x,
                w * o.z + x * o.y - y * o.x + z * o.w,
                w * o.w - x * o.x - y * o.y - z * o.z,
            };
        }

        void normalize()
        {
            float len = std::sqrt(x * x + y * y + z * z + w * w);
            if (len == 0.f) {
                *this = Rotation3D {};
                return;
            }
            x /= len;
            y /= len;
            z /= len;
            w /= len;
        }
    };

} // namespace RType

#endif /* !ROTATION3D_HPP_ */
