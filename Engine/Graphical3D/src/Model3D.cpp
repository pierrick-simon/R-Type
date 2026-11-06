/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Model3D
*/

#include "Model3D.hpp"
#include "Exceptions.hpp"

namespace RType {
    Model3D::Model3D(const std::string &path)
    {
        try {
            this->Load(path);
        } catch (raylib::RaylibException &e) {
            throw(EngineException(e.what()));
        }
    }

    void Model3D::unloadModels3D(GameManager &gm)
    {
        for (auto [e, _] : gm.view<Model3D>().getVector())
            gm.destroyEntity(e);
    }
}; // namespace RType
