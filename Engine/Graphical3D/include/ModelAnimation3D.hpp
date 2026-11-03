/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ModelAnimation3D
*/

#ifndef MODELANIMATION3D_HPP_
#define MODELANIMATION3D_HPP_

#include <raylib-cpp.hpp>
#include <string>
#include <vector>

namespace RType {
    class ModelAnimation3D {
    public:
        explicit ModelAnimation3D(const std::string &path);

        void update(const raylib::Model &model, std::size_t index, int frame);

        [[nodiscard]] std::size_t count() const
        {
            return _animations.size();
        }

        [[nodiscard]] int frameCount(std::size_t index) const;

        [[nodiscard]] const raylib::ModelAnimation &getAnimation(
            int index) const
        {
            return _animations[index % _animations.size()];
        }

    private:
        std::vector<raylib::ModelAnimation> _animations;
    };
} // namespace RType
#endif /* !MODELANIMATION3D_HPP_ */
