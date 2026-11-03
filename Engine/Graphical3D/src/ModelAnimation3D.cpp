/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** ModelAnimation3D
*/

#include "ModelAnimation3D.hpp"
#include "Exceptions.hpp"

#include "Exceptions.hpp"
#include "ModelAnimation3D.hpp"

namespace RType {
    ModelAnimation3D::ModelAnimation3D(const std::string &path)
    {
        try {
            _animations = raylib::ModelAnimation::Load(path);
        } catch (raylib::RaylibException &e) {
            throw EngineException(e.what());
        }
        if (_animations.empty())
            throw EngineException("No animations found in: " + path);
    }

    void ModelAnimation3D::update(
        const raylib::Model &model, std::size_t index, int frame)
    {
        if (index >= _animations.size())
            throw EngineException("Animation index out of range");
        _animations[index].Update(model, frame);
    }

    int ModelAnimation3D::frameCount(std::size_t index) const
    {
        if (index >= _animations.size())
            throw EngineException("Animation index out of range");
        return _animations[index].GetFrameCount();
    }
} // namespace RType
